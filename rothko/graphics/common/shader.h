// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/utils/macros.h"
#include "rothko/utils/clear_on_move.h"

namespace rothko {

struct Renderer;

// Uniforms --------------------------------------------------------------------

enum class UniformType {
  kBool, kInt, kFloat,
  kInt2, kInt3, kInt4,
  kUint2, kUint3, kUint4,
  kVec2, kVec3, kVec4,
  kIvec2, kIvec3, kIvec4,
  kUvec2, kUvec3, kUvec4,
  kMat4,
  kLast,
};
const char* ToString(UniformType);
// Returns kLast if string is not valid.
UniformType FromString(const std::string&);

uint32_t GetSize(UniformType);
uint32_t GetAlignment(UniformType);

// |offset| represents the offset in memory where this uniform is from the
// beginning of the uniform block. This can be calculated calling into
// CalculateUniformLayout with a set of uniforms that have been loaded.
struct Uniform {
  std::string name;
  UniformType type = UniformType::kLast;
  uint32_t alignment = 0;   // In bytes.
  uint32_t offset = 0;      // In bytes.
  uint32_t size = 0;        // In bytes.
  // TODO(Cristian): Support arrays.
};
inline bool Valid(Uniform* u) { return u->type != UniformType::kLast; }

// A UniformBufferObject is a group of uniforms grouped in a struct-ish
// configuration within the shader. The advantage of those is that they can be
// mapped directly from a buffer upload (eg. memcpy) instead of individually
// through glUniform1v kind of calls.
struct UniformBufferObject {
  std::string name;
  int size = -1;  // In bytes.
  std::vector<Uniform> uniforms;
};

// This is using the std140 uniform block layout rules:
//
// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL (Uniform block layout).
// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt
bool CalculateUBOLayout(UniformBufferObject*);

// Mostly exposed for testing.
struct SubShaderParseResult {
  std::vector<UniformBufferObject> ubos;
};
bool ParseSubShader(const std::string& source, SubShaderParseResult* out);

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
