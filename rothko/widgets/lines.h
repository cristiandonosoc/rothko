// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"

#include "rothko/math/math.h"

namespace rothko {

// Appends into a single mesh the set of lines and figures push into it.
// Uses primitive reset to render them all in a single draw call.
struct LineManager {
  std::string name;

  const Shader* shader;

  // How many shapes have been added to the line manager. Will be zeroed upon reset.
  int shape_count = 0;

  bool staged = false;          // Whether the current state of the mesh has been staged.

  // Uses PrimitiveType::kLineStrip.
  // Will be kept up to date according to the current state of |mesh|.
  // IMPORTANT: The GPU state might be out of date. Use |staged| to know this and call |Stage|
  //            accordingly.
  Mesh strip_mesh;

  // You should not access the render command directly, as it might be empty.
  // Use |GetRenderCommand| instead.
  RenderMesh render_command_;
};

std::unique_ptr<Shader> CreateLineShader(Renderer* renderer);

// |line_count| means how many lines we pre-allocate within the mesh.
// Keep in mind that a cube = 10 lines.
bool Init(LineManager*, Renderer*, const Shader*, const std::string& name,
          uint32_t line_count = 1000);
inline bool Valid(LineManager* l) { return Staged(l->strip_mesh); }
void Reset(LineManager*);

bool Stage(LineManager*, Renderer*);

RenderCommand GetRenderCommand(const LineManager&);

// Push primitives ---------------------------------------------------------------------------------

void PushLine(LineManager*, Vec3 from, Vec3 to, Color color);

void PushCubeCenter(LineManager*, Vec3 center, Vec3 extents, Color color);
inline void PushCube(LineManager* lm, Vec3 min, Vec3 max, Color color) {
  PushCubeCenter(lm, (min + max) / 2, Abs((max - min) / 2), color);
}

// Normal will be used to calculate the axis frame (where the right and up vectors are according to
// the |normal|).
void PushRing(LineManager*, Vec3 center, Vec3 normal, float radius, Color color);

void PushRing(LineManager*, Vec3 center, const AxisFrame&, float radius, Color color);

}  // namespace
