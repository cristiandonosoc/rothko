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

// A UniformBufferObject is a group of uniforms grouped in a struct-ish
// configuration within the shader. The advantage of those is that they can be
// mapped directly from a buffer upload (eg. memcpy) instead of individually
// through glUniform1v kind of calls.
struct UniformBufferObject {
  int size = -1;  // In bytes.
  std::vector<Uniform> uniforms;
};

struct Shader {
  RAII_CONSTRUCTORS(Shader);

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;  // Set by the renderer.

  std::vector<UniformBufferObject> vert_ubos;
  std::vector<UniformBufferObject> frag_ubos;

  std::string vert_source;
  std::string frag_source;
};

inline bool Staged(Shader* s) { return s->renderer && s->uuid.has_value(); }

}  // namespace rothko
