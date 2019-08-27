// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/game.h>
#include <rothko/logging/logging.h>

#include "memory.h"
#include "quad.h"
#include "textures.h"

namespace rothko {
namespace emulator {

// The gameboy display is 160x144 (20x18 tiles).
// The background is 32x32 tiles with a sliding window.

struct Display {
  Texture background;
  Texture window;

  QuadManager quads;
};

bool InitDisplay(Game*, Display* out);

std::unique_ptr<Mesh> CreateBackgroundMesh(Game* game);

void CreateBackgroundMesh(Renderer*, Display*, Memory*, Texture*, Shader*, uint8_t* camera_ubo);

bool UpdateBackgroundMesh(Game* game, Memory* memory, Mesh* mesh);
void UpdateBackgroundTexture(Memory* memory, Texture* background_texture);

void CreateDisplayImgui(Memory* memory, Textures* textures);

}  // namespace emulator
}  // namespace rothko
