// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"

namespace rothko {

struct Grid {
  std::string name;

  Mesh mesh;

  RenderMesh render_command;
};

bool Init(Renderer*, Grid*, std::string name);
inline bool Valid(Grid* g) { return Staged(g->mesh); }

}  // namespace

