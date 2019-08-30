// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "textures.h"

#include <rothko/game.h>

#include "display.h"
#include "memory.h"

namespace rothko {
namespace emulator {

// CreateTileTexture -------------------------------------------------------------------------------

namespace {

bool CreateTexture(Game* game, std::string name, Vec2 size, Texture* out) {
  out->name = std::move(name);
  out->type = TextureType::kRGBA;
  out->size = {(int)size.x, (int)size.y};

  size_t alloc_size = sizeof(Color) * size.x * size.y;
  out->data = (uint8_t*)malloc(alloc_size);
  out->free_function = free;

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  if (!RendererStageTexture(game->renderer.get(), out, config))
    return false;
  return true;
}

}  // namespace

std::unique_ptr<Textures> CreateTextures(Game* game) {
  auto textures = std::make_unique<Textures>();
  if (!CreateTexture(game, "tiles-texture", {kTilesSizeX, kTilesSizeY}, &textures->tiles) ||
      !CreateTexture(game, "bg-texture", {kBGSizeX, kBGSizeY}, &textures->background) ||
      !CreateTexture(game, "window-texture", {kWindowSizeX, kWindowSizeY}, &textures->window) ||
      !CreateTexture(game, "sprites-texture", {kSpritesSizeX, kSpritesSizeY}, &textures->sprites) ||
      !CreateTexture(game, "sprites-debug-texture", {kSpritesSizeX, kSpritesSizeY},
                           &textures->sprites_debug)) {
    return nullptr;
  }

  FillInTransparent(&textures->sprites);
  RendererSubTexture(game->renderer.get(), &textures->sprites);

  return textures;
}

// FillInTransparent -------------------------------------------------------------------------------

void FillInTransparent(Texture* texture) {
  constexpr int kSquareSize = 4;

  Color gray = CreateGray(0xdd);

  Color* color = (Color*)texture->data.value;
  Color* end = color + texture->size.x * texture->size.y;
  for (int y = 0; y < texture->size.height; y++) {
    int tile_y = y / kSquareSize;
    for (int x = 0; x < texture->size.width; x++) {
      int tile_x = x / kSquareSize;
      *color++ = ((tile_x + tile_y) % 2 == 0) ? Color::White() : gray;
    }
  }

  ASSERT(color == end);
}

// UpdateTileTexture -------------------------------------------------------------------------------

namespace {

Int2 IndexToCoord(int index) {
  return {index % kTileTextureCountX, (index / kTileTextureCountY)};
}

int CoordToIndex(Int2 coord) {
  return coord.y * kTileTextureCountX + coord.x;
}

void PaintTile(Color* data, Int2 coord, Color color) {
  int cx = coord.x * kTileSizeX;
  int cy = coord.y * kTileSizeY;

  for (int y = cy; y < cy + kTileSizeY; y++) {
    for (int x = cx; x < cx + kTileSizeX; x++) {
      data[y * kTilesSizeX+ x] = color;
    }
  }
}

void PaintTile(Color* data, int index, Color color) {
  PaintTile(data, IndexToCoord(index), color);
}

void PaintTile(Color* data, Int2 coord, const Color* tile_data) {
  Color* tile_base = data + (coord.y * kTileSizeX * kTilesSizeX  + coord.x * kTileSizeX);
  for (int y = 0; y < 8; y++) {
    Color* row_base = tile_base + (y * kTilesSizeX);
    for (int x = 0; x < 8; x++) {
      row_base[x] = *tile_data++;
    }
  }
}

void PaintTilePixelOffset(Texture* texture, Int2 pos, const Color* tile_data, bool debug) {
  (void)tile_data;
  Color* base = (Color*)texture->data.value;
  Color* end = base + texture->size.width * texture->size.height;

  Color* sprite_base = base + pos.y * texture->size.width + pos.x;
  for (int y = 0; y < kTileSizeY; y++) {
    // The sprite might be offset such that it's only partially seen.
    // We need to detect that case.
    Color* ptr = sprite_base + (y * texture->size.width);
    if (ptr >= end)
      break;

    for (int x = 0; x < kTileSizeX; x++) {
      // We always advance the pointers, even if we don't draw in debug mode.
      Color* dst = ptr++;
      const Color* src = tile_data++;

      if (debug && IsTransparent(*src))
        continue;

      *dst = *src;
      if (ptr >= end)
        break;
    }
  }
}

void PaintTile(Color* data, int index, const Color* tile_data) {
  PaintTile(data, IndexToCoord(index), tile_data);
}

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
void TileToTexture(uint8_t palette, const void* data, Color* out, bool sprite) {
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
      uint8_t shade = msp << 1 | lsp;
      ASSERT_MSG(shade < 0b100, "Got shade: 0x%x", shade);

      // Sprites use color 0 as transparent.
      if (sprite && shade == 0) {
        *out = Color::Transparent();
      } else {
        *out = ShadeToColor(shades[shade]);
      }
      out++;
    }

    ptr += 2;
  }
}

}  // namespace

void UpdateTileTexture(Memory* memory, Texture* tile_texture) {
  // Fill in the texture.
  Color tile_color[64];
  Color* base_color = (Color*)tile_texture->data.value;

  for (int y = 0; y < kTileTextureCountY; y++) {
    for (int x = 0; x < kTileTextureCountX; x++) {
      Tile* tile = memory->vram.tiles + (y * kTileTextureCountX) + x;
      TileToTexture(memory->mapped_io.bgp, tile, tile_color, false);
      PaintTile(base_color, {x, y}, tile_color);
    }
  }
}

void UpdateBackgroundTexture(Memory* memory, Texture* texture) {
  int map_index = LCDC_BG_TILE_MAP_DISPLAY_SELECT(memory->mapped_io.lcdc);
  uint8_t tile_data_select = LCDC_BG_WINDOW_TILE_DATA_SELECT(memory->mapped_io.lcdc);

  Color tile_color[64];
  uint8_t* background_map = map_index == 0 ? memory->vram.tilemap0 : memory->vram.tilemap1;

  for (int y = 0; y < kBGTileY; y++) {
    for (int x = 0; x < kBGTileX; x++) {
      // Map 0 maps [0, 256).
      // Map 1 maps [-128, 128) (signed).
      uint8_t tile_index = background_map[y * kTileMapCountX + x];
      int index = tile_data_select == 0 ? tile_index : (int)tile_index;

      Tile* tile = memory->vram.tiles + index;
      TileToTexture(memory->mapped_io.bgp, tile, tile_color, false);
      PaintTilePixelOffset(texture, {x * kTileSizeX, y * kTileSizeY}, tile_color, false);
    }
  }
}

void UpdateWindowTexture(Memory* memory, Texture* texture) {
  int map_index = LCDC_WINDOW_TILE_MAP_DISPLAY_SELECT(memory->mapped_io.lcdc);
  uint8_t tile_data_select = LCDC_BG_WINDOW_TILE_DATA_SELECT(memory->mapped_io.lcdc);

  Color tile_color[64];
  uint8_t* window_map = map_index == 0 ? memory->vram.tilemap0 : memory->vram.tilemap1;

  for (int y = 0; y < kWindowTileY; y++) {
    for (int x = 0; x < kWindowTileX; x++) {
      // Map 0 maps [0, 256).
      // Map 1 maps [-128, 128) (signed).
      uint8_t tile_index = window_map[y * kWindowTileX + x];
      int index = tile_data_select == 0 ? tile_index : (int)tile_index;

      Tile* tile = memory->vram.tiles + index;
      TileToTexture(memory->mapped_io.bgp, tile, tile_color, false);
      PaintTilePixelOffset(texture, {x * kTileSizeX, y * kTileSizeY}, tile_color, false);
    }
  }
}

void UpdateSpritesDebugTexture(Memory* memory, Texture* texture) {
  FillInTransparent(texture);

  Color sprite_color[64];
  for (OAMEntry& sprite : memory->oam_table) {
    if (SpriteIsHidden(memory, sprite))
        continue;

    Tile* tile = memory->vram.tiles + sprite.tile_number;
    TileToTexture(memory->mapped_io.bgp, tile, sprite_color, true);
    PaintTilePixelOffset(texture, {sprite.x + 8, sprite.y + 8}, sprite_color, true);
  }
}

}  // namespace emulator
}  // namespace rothko
