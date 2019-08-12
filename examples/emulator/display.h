// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/logging/logging.h>
#include <rothko/game.h>

#include "memory.h"
#include "quad.h"

namespace rothko {
namespace emulator {

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

struct Display {
  Texture background;
  Texture window;

  QuadManager quads;
};

bool InitDisplay(Game*, Display* out);

// |palette| is a palette register (bgp, obp0, obp1).
// |data| are 16 bytes, representing 64 pixels of 2 bits each.
// |out| must be able to support 8x8 pixels (64).
void TileToTexture(uint8_t palette, const void* data, Color* out);

std::unique_ptr<Mesh> CreateBackgroundMesh(Game* game);

void CreateBackgroundMesh(Renderer*, Display*, Memory*, Texture*, Shader*, uint8_t* camera_ubo);

bool UpdateBackgroundMesh(Game* game, Memory* memory, Mesh* mesh);


void CreateDisplayImgui(Memory* memory, Texture* tilemap);


void UpdateBackgroundTexture(Memory* memory, Texture* background_texture);

}  // namespace emulator
}  // namespace rothko
