// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/utils/macros.h"
#include "rothko/utils/clear_on_move.h"

namespace rothko {

struct Renderer;

struct Shader {
  // A UniformBufferObject is a group of uniforms grouped in a struct-ish configuration within the
  // shader. The advantage of those is that they can be mapped directly from a buffer upload
  // (eg. memcpy) instead of individually through glUniform1v kind of calls.
  struct UBO {
    std::string name;
    uint32_t size = 0;  // In bytes.
  };

  RAII_CONSTRUCTORS(Shader);

  std::string name;

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;  // Set by the renderer.

  UBO vert_ubo;
  UBO frag_ubo;

  std::string vert_src;
  std::string frag_src;
};

inline bool Valid(const Shader::UBO& ubo) { return ubo.size != 0; }

inline bool Loaded(const Shader& s) { return !s.vert_src.empty() && !s.frag_src.empty(); }
inline bool Staged(const Shader& s) { return s.renderer && s.uuid.has_value(); }

bool LoadShaderSources(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out);

}  // namespace rothko
