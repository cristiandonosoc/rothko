// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "display.h"

#include <rothko/logging/logging.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/defer.h>
#include <rothko/utils/strings.h>

namespace rothko {
namespace emulator {

// Each pixels are defined by two bytes, where one is the "upper index" of the pixel and the second
// is the lower pixel. These are shades that need to be interpreted into a color according to a
// palette register (bgp, obp0, obp1).
//
// |7|6|5|4|3|2|1|0| Byte 1 (Least significant bit).
// |7|6|5|4|3|2|1|0| Byte 2 (Most significant bit).
//  | | | | | | | |
//  | | | | | | | |-> Shade 0
//  | | | | | | |---> Shade 1
//  | | | | | |-----> Shade 2
//  | | | | |-------> Shade 3
//  | | | |---------> Shade 4
//  | | |-----------> Shade 5
//  | |-------------> Shade 6
//  |---------------> Shade 7
void TileToTexture(uint8_t palette, const void* data, Color* out) {
  uint32_t shades[4] = {
    PaletteColor(palette, 0),
    PaletteColor(palette, 1),
    PaletteColor(palette, 2),
    PaletteColor(palette, 3),
  };

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

      *out = ShadeToColor(shades[pixel]);
      out++;
    }

    ptr += 2;
  }
}

// Background Mesh --------------------------------------------------------------------------------

namespace {

int kTileSize = 8;
Int2 kTileCount =  {16, 16 + 8};
Int2 kTextureDim = kTileCount * kTileSize;
Vec2 kUVOffset = {1.0f / 16.0f, 1.0f / (16.0f + 8.0f)};

Vertex3dUVColor CreateVertex(Vec3 pos, Vec2 uv, Color color) {
  Vertex3dUVColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = ToUint32(color);

  return vertex;
}

void PushSquare(Mesh* mesh, Vec2 base, Vec2 size, Vec2 uv_base) {
  Vertex3dUVColor vertices[] = {
      CreateVertex({base.x, base.y, 0}, uv_base, colors::kWhite),
      CreateVertex({base.x, base.y + size.y, 0}, uv_base + Vec2{kUVOffset.x, 0.0f}, colors::kWhite),
      CreateVertex({base.x + size.x, base.y, 0}, uv_base + Vec2{0, kUVOffset.y}, colors::kWhite),
      CreateVertex({base.x + size.x, base.y + size.y, 0}, uv_base + kUVOffset, colors::kWhite),
  };

  Mesh::IndexType base_index = mesh->vertices_count;

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



}  // namespace

bool UpdateBackgroundMesh(Game* game, Memory* memory, Mesh* mesh) {
  ClearVertices(mesh);
  ClearIndices(mesh);

  for (int y = 0; y < 32; y++) {
    float offset_y = y * (kSize + kBorder);
    for (int x = 0; x < 32; x++) {
      float offset_x = x * (kSize + kBorder);
      uint8_t index = memory->vram.tilemap0[y * 32 + x];
      Vec2 uv_base = TileIndexToUV(index, 0);
      PushSquare(mesh, {offset_x, offset_y}, {kSize, kSize}, uv_base);
    }
  }

  if (!RendererUploadMeshRange(&game->renderer, mesh))
    return false;
  return true;
}

std::unique_ptr<Mesh> CreateBackgroundMesh(Game* game) {
  auto background_mesh = std::make_unique<Mesh>();
  background_mesh->name = "background";
  background_mesh->vertex_type = VertexType::k3dUVColor;

  for (int y = 0; y < 32; y++) {
    float offset_y = y * (kSize + kBorder);
    for (int x = 0; x < 32; x++) {
      float offset_x = x * (kSize + kBorder);
      PushSquare(background_mesh.get(), {offset_x, offset_y}, {kSize, kSize}, {});
    }
  }

  if (!RendererStageMesh(&game->renderer, background_mesh.get()))
    return nullptr;
  return background_mesh;
}

namespace {


void ShowBackgroundTiles(Memory* memory, Texture* tilemap) {
  ImGui::Text("Background");

  static bool show_indices_inline = false;

  ImGui::Checkbox("Show indices inline", &show_indices_inline);

  // Which background map we're reading from.
  static int map_index = 0;
  ImGui::RadioButton("Background Map 0", &map_index, 0); ImGui::SameLine();
  ImGui::RadioButton("Background Map 1", &map_index, 1);

  // Create tiles.
  constexpr float kImageSize = 30;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t* background_map = map_index == 0 ? memory->vram.tilemap0 :
                                                 memory->vram.tilemap1;

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

      if (ImGui::IsMouseHoveringRect(pos, end)) {
        ImGui::BeginTooltip();

        ImGui::Text("Tile (%02d, %02d) -> %03d", x, y , index);
        ImGui::Image(tilemap, {100, 100}, ToImVec(uv_base), ToImVec(uv_end));

        ImGui::EndTooltip();
      }

      if (show_indices_inline) {
        auto text = StringPrintf("%03d", index);
        draw_list->AddText(ImVec2{pos.x, pos.y + kImageSize - 13}, IM_COL32_WHITE, text.c_str());
      }
    }
  }

  ImGui::Dummy({33 * (kImageSize + 1), 33 * (kImageSize + 1)});
}


}  // namespace

void CreateDisplayImgui(Memory* memory, Texture* tilemap) {
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
  ImGui::Text("BG tilemap: %d", LCDC_BG_TILE_MAP_DISPLAY_SELECT(lcdc) ? 0 : 1);

  ImGui::RadioButton("Window enable", LCDC_WINDOW_DISPLAY_ENABLE(lcdc)); ImGui::SameLine();
  ImGui::Text("Window tilemap: %d", LCDC_WINDOW_TILE_MAP_DISPLAY_SELECT(lcdc) ? 0 : 1);

  ImGui::RadioButton("SpriteEnable", LCDC_OBJ_SPRITE_ENABLE(lcdc)); ImGui::SameLine();
  ImGui::Text("Sprite Size: %s", LCDC_OBJ_SPRITE_SIZE(lcdc) ? "8x8" : "8x16");

  if (ImGui::BeginTabBar("GB Tiles")) {
    if (ImGui::BeginTabItem("Background")) {
      ShowBackgroundTiles(memory, tilemap);
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Window")) {
      ImGui::Text("TODO");
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Sprites")) {
      ImGui::Text("TODO");
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

}

}  // namespace emulator
}  // namespace rothko
