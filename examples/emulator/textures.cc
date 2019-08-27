// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "textures.h"

#include <rothko/game.h>

#include "memory.h"

namespace rothko {
namespace emulator {

// CreateTileTexture -------------------------------------------------------------------------------

namespace {

bool CreateTileTexture(Game* game, Texture* texture) {
  texture->name = "background texture";
  texture->type = TextureType::kRGBA;
  texture->dims = {kTextureDimX, kTextureDimY};

  size_t size = sizeof(Color) * kTextureDimX * kTextureDimY;
  texture->data = (uint8_t*)malloc(size);
  texture->free_function = free;

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  if (!RendererStageTexture(game->renderer.get(), texture, config))
    return false;
  return true;
}

std::unique_ptr<Texture> CreateTransparentTexture(Game* game) {
  auto texture = std::make_unique<Texture>();

  texture->name = "transparent-texture";
  texture->type = TextureType::kRGBA;
  texture->dims = {8, 8};

  size_t size = sizeof(Color) * texture->dims.x * texture->dims.y;
  texture->data = (uint8_t*)malloc(size);
  texture->free_function = free;

  // Fill up texture.
  constexpr int kSquareSize = 4;
  Color* base = (Color*)texture->data.value;
  for (int y = 0; y < texture->dims.y; y++) {
    int kTileY = y / kSquareSize;
    for (int x = 0; x < texture->dims.x; x++) {
      int kTileX = x / kSquareSize;
      *base++ = ((kTileX + kTileY) % 2 == 0) ? colors::kRed : colors::kBlue;
    }
  }

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  config.wrap_u = StageTextureConfig::Wrap::kRepeat;
  config.wrap_v = StageTextureConfig::Wrap::kRepeat;
  if (!RendererStageTexture(game->renderer.get(), texture.get(), config))
    return nullptr;
  return texture;
}

}  // namespace

std::unique_ptr<Textures> CreateTextures(Game* game) {
  auto textures = std::make_unique<Textures>();
  if (!CreateTileTexture(game, &textures->tiles))
    return nullptr;
  return textures;
}

// UpdateTileTexture -------------------------------------------------------------------------------

namespace {

Int2 IndexToCoord(int index) {
  return {index % kTileCountX, (index / kTileCountY)};
}

int CoordToIndex(Int2 coord) {
  return coord.y * kTileCountX + coord.x;
}

void PaintTile(Color* data, Int2 coord, Color color) {
  int cx = coord.x * kTileSizeX;
  int cy = coord.y * kTileSizeY;

  for (int y = cy; y < cy + kTileSizeY; y++) {
    for (int x = cx; x < cx + kTileSizeX; x++) {
      data[y * kTextureDimX+ x] = color;
    }
  }
}

void PaintTile(Color* data, int index, Color color) {
  PaintTile(data, IndexToCoord(index), color);
}

void PaintTile(Color* data, Int2 coord, const Color* tile_data) {
  Color* tile_base = data + (coord.y * kTileSizeX * kTextureDimX  + coord.x * kTileSizeX);
  for (int y = 0; y < 8; y++) {
    Color* row_base = tile_base + (y * kTextureDimX);
    for (int x = 0; x < 8; x++) {
      row_base[x] = *tile_data++;
    }
  }
}

void PaintTile(Color* data, int index, const Color* tile_data) {
  PaintTile(data, IndexToCoord(index), tile_data);
}

// Transforms a GB shade (defined in a palette) to a Rothko Color. shades are 2 bits.
inline Color ShadeToColor(uint32_t shade) {
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

}  // namespace

void UpdateTileTexture(Game* game, Memory* memory, Texture* tile_texture) {
  // Fill in the texture.
  Color tile_color[64];
  Color* base_color = (Color*)tile_texture->data.value;

  for (int y = 0; y < 16 + 8; y++) {
    for (int x = 0; x < 16; x++) {
      Tile* tile = memory->vram.tiles + (y * 16) + x;
      TileToTexture(memory->mapped_io.bgp, tile, tile_color);
      PaintTile(base_color, {x, y}, tile_color);
    }
  }

  LOG(App, "Updating texture");
  RendererSubTexture(game->renderer.get(), tile_texture, {0, 0}, {kTextureDimX, kTextureDimY},
                     tile_texture->data.value);
}

}  // namespace emulator
}  // namespace rothko
