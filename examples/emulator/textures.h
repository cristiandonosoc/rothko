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

// Tile Texture
constexpr int kTilesSizeX = kTileCountX * kTileSizeX;
constexpr int kTilesSizeY = kTileCountY * kTileSizeY;
constexpr float kTilesUVOffsetX = 1.0f / (float)kTileCountX;
constexpr float kTilesUVOffsetY = 1.0f / (float)kTileCountY;

// We render the whole background and decide which part to show.
constexpr int kBGSizeX = 32 * kTileSizeX;
constexpr int kBGSizeY = 32 * kTileSizeY;

constexpr int kWindowSizeX = 20 * kTileSizeX;
constexpr int kWindowSizeY = 18 * kTileSizeY;

constexpr int kSpritesSizeX = 20 * kTileSizeX;
constexpr int kSpritesSizeY = 18 * kTileSizeY;

// Textures ----------------------------------------------------------------------------------------

struct Textures {
  Texture tiles;
  Texture background;
  Texture window;
  Texture sprites;
  Texture sprites_debug;
};

std::unique_ptr<Textures> CreateTextures(Game*);

void UpdateTileTexture(Game*, Memory*, Texture*);

// Will override the contents of the texture with a "transparent" pattern.
// This WILL NOT re-upload to the renderer.
void FillInTransparent(Texture*);

void UpdateSpritesDebugTexture(Memory*, Texture*);
void UpdateSpritesTexture(Texture*);

}  // namespace emulator
}  // namespace rothko
