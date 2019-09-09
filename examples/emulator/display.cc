// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "display.h"

#include <rothko/logging/logging.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/defer.h>
#include <rothko/utils/strings.h>

#include "textures.h"

using namespace ::rothko::imgui;

namespace rothko {
namespace emulator {

namespace {

// Constants.

int kTileSize = 8;
Int2 kTileCount =  {16, 16 + 8};
Int2 kTextureDim = kTileCount * kTileSize;
Vec2 kUVOffset = {1.0f / 16.0f, 1.0f / (16.0f + 8.0f)};

}  // namespace

Color ShadeToColor(uint32_t shade) {
  switch (shade) {
    case 0: return Color{0xffffffff};   // White.
    case 1: return Color{0xbbbbbbbb};   // Light gray.
    case 2: return Color{0xff666666};   // Dark gray.
    case 3: return Color{0xff000000};   // Black.
    default: break;
  }

  NOT_REACHED();
  return {};
}

// Display -----------------------------------------------------------------------------------------

bool InitDisplay(Game* game, Display* out) {
  // Init the background texture.
  out->background.name = "background-texture";
  out->background.type = TextureType::kRGBA;
  out->background.size = kTextureDim;

  size_t size = sizeof(Color) * kTextureDim.width * kTextureDim.height;
  out->background.data = (uint8_t*)malloc(size);
  out->background.free_function = free;

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  if (!RendererStageTexture(game->renderer.get(), &out->background, config))
    return false;

  // Init the quad manager (which we will use to output textured quads).

  QuadManagerConfig quad_config;
  quad_config.name = "gb-quads";
  quad_config.capacity = 2000;
  if (!Init(game->renderer.get(), &out->quads, std::move(quad_config)))
    return 1;

  return true;
}

// OnDisplayIO -------------------------------------------------------------------------------------

void OnDisplayIO(Gameboy* gameboy, uint64_t address) {
  (void)gameboy;
  (void)address;
  /* this.display.HandleMemoryChange((MMR)address, value); */
}

// Background Mesh ---------------------------------------------------------------------------------

namespace {

Vertex3dUVColor CreateVertex(Vec3 pos, Vec2 uv, Color color = Color::White()) {
  Vertex3dUVColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = ToUint32(color);

  return vertex;
}

void PushSquare(Mesh* mesh, Vec2 base, Vec2 size, Vec2 uv_base) {
  Vertex3dUVColor vertices[] = {
      CreateVertex({base.x, base.y, 0}, uv_base),
      CreateVertex({base.x, base.y + size.y, 0}, uv_base + Vec2{kUVOffset.x, 0.0f}),
      CreateVertex({base.x + size.x, base.y, 0}, uv_base + Vec2{0, kUVOffset.y}),
      CreateVertex({base.x + size.x, base.y + size.y, 0}, uv_base + kUVOffset),
  };

  Mesh::IndexType base_index = mesh->vertex_count;
  Mesh::IndexType indices[] = {
    0, 1, 2, 2, 1, 3,
    4, 5, 6, 6, 5, 7,
  };
  for (auto& index : indices) {
    index += base_index;
  }

  PushVertices(mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(mesh, indices, ARRAY_SIZE(indices));
}

constexpr int kSize = 40;
constexpr int kBorder = 3;

Vec2 TileIndexToUV(uint8_t index, int map_index) {
  int x = index % 16;
  int y = index / 16;

  // |map_index| == 1 means that we're using background tile map 1, which points uses a different
  // offset scheme and base for finding tiles. See VRAM in memory.h.
  if (map_index == 1)
    y += 16;

  return {x * kUVOffset.x, y * kUVOffset.y};
}

std::pair<Vec2, Vec2> ObtainTileUVs(Memory* memory, Int2 coord, int map_index) {
  uint8_t* background_map = map_index == 0 ? memory->vram.tilemap0 : memory->vram.tilemap1;

  // Background map 0 maps [0, 256).
  // Background map 1 maps [-128, 128).
  int index = 0;
  if (map_index == 0) {
    index = background_map[coord.y * 32 + coord.x];
  } else if (map_index == 1) {
    index = (int)(background_map[coord.y * 32 + coord.x]);
  }

  Vec2 uv_base = TileIndexToUV(index, map_index);
  Vec2 uv_end = uv_base + kUVOffset;

  return {uv_base, uv_end};
}

}  // namespace

bool UpdateBackgroundMesh(Game* game, Memory* memory, Mesh* mesh) {
  Reset(mesh);

  for (int y = 0; y < 32; y++) {
    float offset_y = y * (kSize + kBorder);
    for (int x = 0; x < 32; x++) {
      float offset_x = x * (kSize + kBorder);
      uint8_t index = memory->vram.tilemap0[y * 32 + x];
      Vec2 uv_base = TileIndexToUV(index, 0);
      PushSquare(mesh, {offset_x, offset_y}, {kSize, kSize}, uv_base);
    }
  }

  if (!RendererUploadMeshRange(game->renderer.get(), mesh))
    return false;
  return true;
}

void CreateBackgroundMesh(Renderer* renderer, Display* display, Memory* memory, Texture* texture,
                          Shader* shader, uint8_t* camera) {
  QuadEntry entry;
  entry.shader = shader;
  entry.texture = texture;
  entry.vert_ubo = camera;

  for (int y = 0; y < 32; y++) {
    float offset_y = y * (kSize + kBorder);
    for (int x = 0; x < 32; x++) {
      float offset_x = x * (kSize + kBorder);

      entry.from_pos = {offset_x, offset_y, 0.0f};
      entry.to_pos = entry.from_pos + Vec3{kSize, kSize, 0.0f};

      auto [from_uv, to_uv] =
          ObtainTileUVs(memory, {x, y}, LCDC_BG_TILE_MAP_DISPLAY_SELECT(memory->mapped_io.lcdc));
      entry.from_uv = from_uv;
      entry.to_uv = to_uv;

      Push(&display->quads, entry);

      break;
    }

    break;
  }

  Stage(renderer, &display->quads);

  LOG(App, "Created background mesh!");
}

namespace {

// |out| is uint8_t[64];
void TileToIndexes(const void* data, uint8_t* out) {
  const uint8_t* ptr = (uint8_t*)data;
  for (int y = 0; y < 8; y++) {
    const uint8_t* lsb = ptr + 0;
    const uint8_t* msb = ptr + 1;

    // Bit 7 is the left-most pixel, so we iterate it backwards.
    for (int x = 7; x >= 0; x--) {
      uint8_t lsp = (*lsb >> x) & 0x1;
      uint8_t msp = (*msb >> x) & 0x1;
      uint8_t pixel = msp << 1 | lsp;
      ASSERT_MSG(pixel < 0b100, "Got pixel: 0x%x", pixel);

      *out++ = pixel;
    }

    ptr += 2;
  }
}

void PixelRowToImguiText(uint8_t* p) {
  ImGui::Text("%u|%u|%u|%u|%u|%u|%u|%u", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

void TileTooltip(Memory* memory, Texture* tilemap, Int2 pos, Vec2 uv_base, Vec2 uv_end, int index) {
  ImGui::BeginTooltip();
  ImGui::Text("Tile (%02d, %02d) -> %03d", pos.x, pos.y, index);

  float size = 100.0f;
  ImGui::BeginChild("tooltip-1", {size + 5, size}, false);
  ImGui::Image(tilemap, {size, size}, ToImVec2(uv_base), ToImVec2(uv_end));
  ImGui::EndChild();

  ImGui::SameLine();
  ImGui::BeginChild("tooltip-2", {120, 160}, true);

  uint8_t p[64];
  TileToIndexes((void*)&memory->vram.tiles[index], p);


  PixelRowToImguiText(p + 0 * 8);
  PixelRowToImguiText(p + 1 * 8);
  PixelRowToImguiText(p + 2 * 8);
  PixelRowToImguiText(p + 3 * 8);
  PixelRowToImguiText(p + 4 * 8);
  PixelRowToImguiText(p + 5 * 8);
  PixelRowToImguiText(p + 6 * 8);
  PixelRowToImguiText(p + 7 * 8);
  ImGui::EndChild();

  ImGui::EndTooltip();
}

void ShowTiles(Memory* memory, Texture* tilemap, Int2 size, int map_index, bool indices_inline) {
  // Create tiles.
  constexpr float kImageSize = 30;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  uint8_t* background_map = map_index == 0 ? memory->vram.tilemap0 : memory->vram.tilemap1;
  for (int y = 0; y < size.y; y++) {
    for (int x = 0; x < size.x; x++) {

      // Background map 0 maps [0, 256).
      // Background map 1 maps [-128, 128).
      int index = 0;
      if (map_index == 0) {
        index = background_map[y * 32 + x];
      } else if (map_index == 1) {
        index = (int)(background_map[y * 32 + x]);
      }

      Vec2 uv_base = TileIndexToUV(index, map_index);
      Vec2 uv_end = uv_base + kUVOffset;

      Vec2 pos = ImGui::GetCursorScreenPos();
      pos.x += x * (kImageSize + 1);
      pos.y += y * (kImageSize + 1);

      Vec2 end = pos + Vec2{kImageSize, kImageSize};

      draw_list->AddImageQuad(tilemap,
                              pos,{pos.x, end.y},
                              end, {end.x, pos.y},
                              uv_base, {uv_base.x, uv_end.y}, uv_end, {uv_end.x, uv_base.y});

      if (ImGui::IsMouseHoveringRect(pos, end))
        TileTooltip(memory, tilemap, {x, y}, uv_base, uv_end, index);

      if (indices_inline) {
        auto text = StringPrintf("%03d", index);
        draw_list->AddText(ImVec2{pos.x, pos.y + kImageSize - 13}, IM_COL32_WHITE, text.c_str());
      }
    }
  }

  ImGui::Dummy({size.x * (kImageSize + 1), size.y * (kImageSize + 1)});
}

void ShowBackgroundTiles(Memory* memory, Textures* textures) {
  ImGui::Text("Background");

  static bool indices_inline = false;

  ImGui::Image(&textures->background, ToImVec2(textures->background.size * 2));

  ImGui::Separator();

  ImGui::Checkbox("Show indices inline", &indices_inline);

  static int map_index = 0;
  static int initial_map_index = 0;
  int actual_map_index = LCDC_BG_TILE_MAP_DISPLAY_SELECT(memory->mapped_io.lcdc) ? 1 : 0;
  if (initial_map_index != actual_map_index) {
    initial_map_index = actual_map_index;
    map_index = initial_map_index;
  }

  // Which background map we're reading from.
  ImGui::RadioButton("Background Map 0", &map_index, 0); ImGui::SameLine();
  ImGui::RadioButton("Background Map 1", &map_index, 1);

  ShowTiles(memory, &textures->tiles, {32, 32}, map_index, indices_inline);
}

void ShowWindowTiles(Memory* memory, Textures* textures) {
  ImGui::Text("Window");

  ImGui::Image(&textures->window, ToImVec2(textures->window.size * 2));

  static bool indices_inline = false;
  ImGui::Checkbox("Show indices inline", &indices_inline);

  static int map_index = 0;
  static int initial_map_index = 0;
  int actual_map_index = LCDC_BG_TILE_MAP_DISPLAY_SELECT(memory->mapped_io.lcdc) ? 1 : 0;
  if (initial_map_index != actual_map_index) {
    initial_map_index = actual_map_index;
    map_index = initial_map_index;
  }

  // Which background map we're reading from.
  ImGui::RadioButton("Background Map 0", &map_index, 0); ImGui::SameLine();
  ImGui::RadioButton("Background Map 1", &map_index, 1);

  ShowTiles(memory, &textures->tiles, {20, 18}, map_index, indices_inline);
}

void ShowSpriteTiles(Memory* memory, Textures* textures) {
  ImGui::Text("Sprites");

  // Show the target image.
  ImGui::Image(&textures->sprites_debug, ToImVec2(textures->sprites_debug.size * 3));

  ImGui::Separator();

  // Show all the tiles that compose the image.

  constexpr float kImageSize = 30;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  static bool indices_inline = false;
  ImGui::Checkbox("Show indices inline", &indices_inline);

  Int2 size = {10, 4};
  for (int y = 0; y < size.y; y++) {
    for (int x = 0; x < size.x; x++) {
      OAMEntry& entry = memory->oam_table[x + y * size.x];

      int index = entry.tile_number;
      Vec2 uv_base = TileIndexToUV(index, 0);
      Vec2 uv_end = uv_base + kUVOffset;

      Vec2 pos = ImGui::GetCursorScreenPos();
      pos.x += x * (kImageSize + 1);
      pos.y += y * (kImageSize + 1);

      Vec2 end = pos + Vec2{kImageSize, kImageSize};

      draw_list->AddImageQuad(&textures->tiles,
                              pos, {pos.x, end.y}, end, {end.x, pos.y},
                              uv_base, {uv_base.x, uv_end.y}, uv_end, {uv_end.x, uv_base.y});
    if (ImGui::IsMouseHoveringRect(pos, end))
        TileTooltip(memory, &textures->tiles, {x, y}, uv_base, uv_end, index);

      if (indices_inline) {
        auto text = StringPrintf("%03d", index);
        draw_list->AddText(ImVec2{pos.x, pos.y + kImageSize - 13}, IM_COL32_WHITE, text.c_str());
      }
    }
  }

  ImGui::Dummy({size.x * (kImageSize + 1), size.y * (kImageSize + 1)});
}

void CreateColorPicker(int index, int shade) {
  ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions |
                              ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoLabel;

  ImVec4 color = ToImVec4(ToVec4(ShadeToColor(shade)));
  ImGui::AlignFirstTextHeightToWidgets();
  ImGui::Text("Shade %d (%u)", index, shade); ImGui::SameLine();
  ImGui::ColorEdit3("", (float*)&color, flags);
}

}  // namespace

void CreateDisplayImgui(Memory* memory, Textures* textures) {
  ImGui::Begin("Display", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
  DEFER([]() { ImGui::End(); });

  if (!Loaded(*memory)) {
    ImGui::Text("No ROM loaded.");
    return;
  }

  uint8_t lcdc = memory->mapped_io.lcdc;

  ImGui::Text("LCDC: 0x%x", lcdc);
  ImGui::RadioButton("Display enable", LCDC_DISPLAY_ENABLE(lcdc));

  ImGui::Text("BG/Window Tile base: %s", LCDC_BG_WINDOW_TILE_DATA_SELECT(lcdc) ?
      "[0, 256) tile index is uint8_t" : "[128, 384) tile index is int8_t");

  ImGui::RadioButton("BG Display", LCDC_BG_DISPLAY(lcdc)); ImGui::SameLine();
  ImGui::Text("BG tilemap: %d", LCDC_BG_TILE_MAP_DISPLAY_SELECT(lcdc) ? 1 : 0);

  ImGui::RadioButton("Window enable", LCDC_WINDOW_DISPLAY_ENABLE(lcdc)); ImGui::SameLine();
  ImGui::Text("Window tilemap: %d", LCDC_WINDOW_TILE_MAP_DISPLAY_SELECT(lcdc) ? 1 : 0);

  ImGui::RadioButton("SpriteEnable", LCDC_OBJ_SPRITE_ENABLE(lcdc)); ImGui::SameLine();
  ImGui::Text("Sprite Size: %s", LCDC_OBJ_SPRITE_SIZE(lcdc) ? "8x8" : "8x16");

  auto bgp = memory->mapped_io.bgp;
  CreateColorPicker(0, LCDC_BGP_GET_COLOR0(bgp)); ImGui::SameLine();
  CreateColorPicker(1, LCDC_BGP_GET_COLOR1(bgp)); ImGui::SameLine();
  CreateColorPicker(2, LCDC_BGP_GET_COLOR2(bgp)); ImGui::SameLine();
  CreateColorPicker(3, LCDC_BGP_GET_COLOR3(bgp));

  if (ImGui::BeginTabBar("GB Tiles")) {
    if (ImGui::BeginTabItem("Background")) {
      ShowBackgroundTiles(memory, textures);
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Window")) {
      ShowWindowTiles(memory, textures);
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Sprites")) {
      ShowSpriteTiles(memory, textures);
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
}

}  // namespace emulator
}  // namespace rothko
