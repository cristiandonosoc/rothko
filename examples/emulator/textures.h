// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/graphics.h>

namespace rothko {

struct Game;

namespace emulator {

struct Memory;

// Size definitions --------------------------------------------------------------------------------

constexpr int kTileSizeX = 8;
constexpr int kTileSizeY = 8;

constexpr int kTileCountX = 16;
constexpr int kTileCountY = 16 + 8;

constexpr int kTextureDimX = kTileCountX * kTileSizeX;
constexpr int kTextureDimY = kTileCountY * kTileSizeY;

constexpr float kUVOffsetX = 1.0f / (float)kTileCountX;
constexpr float kUVOffsetY = 1.0f / (float)kTileCountY;

// Textures ----------------------------------------------------------------------------------------

struct Textures {
  Texture tiles;
  Texture background;
  Texture window;
  Texture sprites;
};

std::unique_ptr<Textures> CreateTextures(Game*);

/* std::unique_ptr<Texture> CreateTileTexture(Game*); */
/* std::unique_ptr<Texture> CreateTransparentTexture(Game*); */

void UpdateTileTexture(Game*, Memory*, Texture*);

}  // namespace emulator
}  // namespace rothko
