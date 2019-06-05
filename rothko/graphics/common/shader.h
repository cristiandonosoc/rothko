// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/utils/macros.h"
#include "rothko/utils/clear_on_move.h"

namespace rothko {

struct Renderer;

struct Uniform {
  // TODO(Cristian): Fill in.
};

struct Uniforms {
  int size = -1;
  std::vector<Uniform> uniforms;
};

struct Shader {
  RAII_CONSTRUCTORS(Shader);

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;  // Set by the renderer.

  Uniforms vert_uniforms;
  Uniforms frag_uniforms;

  std::string vert_src;
  std::string frag_src;
};

inline bool Staged(Shader* s) { return s->renderer && s->uuid.has_value(); }

}  // namespace rothko
