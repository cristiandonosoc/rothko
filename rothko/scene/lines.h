// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"

namespace rothko {

struct LineManager {
  std::string name;
  Mesh mesh;
  Shader shader;

  uint32_t line_count = 0;

  bool staged = false;          // Whether the current state of the mesh has been staged.

  // Will be kept up to date according to the current state of |mesh|.
  // IMPORTANT: The GPU state might be out of date. Use |staged| to know this and call |Stage|
  //            accordingly.
  RenderMesh render_command;    // Will be kept up to date according to the
};

// |line_count| means how many lines we pre-allocate within the mesh.
// Keep in mind that a cube = 10 lines.
bool Init(Renderer*, LineManager*, std::string name, uint32_t line_count = 1000);
inline bool Valid(LineManager* lm) { return Staged(&lm->mesh); }
void Reset(LineManager*);

bool Stage(Renderer*, LineManager*);

// Push primitives ---------------------------------------------------------------------------------

void PushLine(LineManager*, Vec3 from, Vec3 to, Color color);

}  // namespace
