// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "display.h"

#include <rothko/logging/logging.h>

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

Vec2 TileIndexToUV(uint8_t index) {
  int x = index % 16;
  int y = index / 16;

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
      uint8_t index = memory->vram.background_map0[y * 32 + x];
      Vec2 uv_base = TileIndexToUV(index);
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



}  // namespace emulator
}  // namespace rothko
