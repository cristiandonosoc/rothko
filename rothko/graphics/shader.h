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
  RAII_CONSTRUCTORS(Shader);

  std::string name;

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;  // Set by the renderer.

  // A UniformBufferObject is a group of uniforms grouped in a struct-ish configuration within the
  // shader. The advantage of those is that they can be mapped directly from a buffer upload
  // (eg. memcpy) instead of individually through glUniform1v kind of calls.
  //
  // NOTE: The name *must* match the uniform block name within the shader.
  //       Otherwise staging the shader will fail.
  std::string vert_ubo_name;
  std::string frag_ubo_name;
  uint32_t vert_ubo_size = 0;   // In bytes.
  uint32_t frag_ubo_size = 0;   // In bytes.

  uint32_t texture_count = 0;

  std::string vert_src;
  std::string frag_src;
};
inline bool Loaded(const Shader& s) { return !s.vert_src.empty() && !s.frag_src.empty(); }
inline bool Staged(const Shader& s) { return s.renderer && s.uuid.has_value(); }

bool LoadShaderSources(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out);
void RemoveSources(Shader* s);

}  // namespace rothko
