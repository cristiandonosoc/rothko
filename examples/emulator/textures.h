// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/graphics.h>

#include "display.h"

namespace rothko {

struct Game;

namespace emulator {

struct Memory;

// Size definitions --------------------------------------------------------------------------------

// Tile Texture
constexpr int kTileTextureCountX = 16;
constexpr int kTileTextureCountY = 16 + 8;

constexpr int kTilesSizeX = kTileTextureCountX * kTileSizeX;
constexpr int kTilesSizeY = kTileTextureCountY * kTileSizeY;
constexpr float kTilesUVOffsetX = 1.0f / (float)kTileTextureCountX;
constexpr float kTilesUVOffsetY = 1.0f / (float)kTileTextureCountY;

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

// Update Texture Functions ------------------------------------------------------------------------
//
// These functions WILL NOT re-upload to the renderer. That has to be done by the caller.

// Will override the contents of the texture with a "transparent" pattern.
void FillInTransparent(Texture*);

void UpdateTileTexture(Memory*, Texture*);
void UpdateBackgroundTexture(Memory*, Texture*);
void UpdateWindowTexture(Memory*, Texture*);
void UpdateSpritesTexture(Texture*);
void UpdateSpritesDebugTexture(Memory*, Texture*);

}  // namespace emulator
}  // namespace rothko
