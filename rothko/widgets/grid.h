// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"

namespace rothko {

struct Grid {
  const Shader* shader;

  Mesh mesh;
  RenderMesh render_command;
};

inline bool Valid(const Grid& grid) { return !!grid.shader; }

std::unique_ptr<Shader> CreateGridShader(Renderer*, const std::string& name);

// Will attempt to create a grid from default shaders.
bool Init(Grid*, Renderer*);
bool Init(Grid*, Renderer*, const Shader*);

inline bool Valid(Grid* g) { return Staged(g->mesh); }

}  // namespace

