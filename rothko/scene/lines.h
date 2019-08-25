// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"

namespace rothko {

// Appends into a single mesh the set of lines and figures push into it.
// Uses primitive reset to render them all in a single draw call.
struct LineManager {
  std::string name;
  Shader shader;

  bool staged = false;          // Whether the current state of the mesh has been staged.

  // Uses PrimitiveType::kLineStrip.
  // Will be kept up to date according to the current state of |mesh|.
  // IMPORTANT: The GPU state might be out of date. Use |staged| to know this and call |Stage|
  //            accordingly.
  Mesh strip_mesh;
  RenderMesh render_command;
};

// |line_count| means how many lines we pre-allocate within the mesh.
// Keep in mind that a cube = 10 lines.
bool Init(Renderer*, LineManager*, std::string name, uint32_t line_count = 1000);
inline bool Valid(LineManager* l) { return Staged(&l->strip_mesh); }
void Reset(LineManager*);

bool Stage(Renderer*, LineManager*);

// Push primitives ---------------------------------------------------------------------------------

void PushLine(LineManager*, Vec3 from, Vec3 to, Color color);

void PushCubeCenter(LineManager*, Vec3 center, Vec3 extents, Color color);
inline void PushCube(LineManager* lm, Vec3 min, Vec3 max, Color color) {
  PushCubeCenter(lm, (min + max) / 2, Abs((max - min) / 2), color);
}

}  // namespace
