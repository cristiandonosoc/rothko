// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/shader.h"

#include <map>
#include <optional>

#include "rothko/graphics/common/renderer.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/strings.h"

namespace rothko {

Shader::~Shader() {
  if (Staged(this))
    RendererUnstageShader(this->renderer, this);
}

// ParseSubShader --------------------------------------------------------------

namespace {

std::vector<std::string> GetElements(const std::string& line) {
  auto elements = SplitToLines(line.substr(3), ":");
  if (elements.size() != 3u) {
    LOG(ERROR, "Wrong uniform line: %s", line.c_str());
    return {};
  }

  for (auto& element : elements) {
    element = Trim(element);
  }

  return elements;
}

std::optional<Uniform> ParseUniform(const std::vector<std::string>& elements) {
  ASSERT(elements.size() == 3u);

  Uniform uniform;
  uniform.name = elements[1];

  uniform.type = FromString(elements[2]);
  if (uniform.type == UniformType::kLast) {
    LOG(ERROR, "Wrong uniform type: %s", ToString(uniform.type));
    return std::nullopt;
  }
  uniform.size = GetSize(uniform.type);
  uniform.alignment = GetAlignment(uniform.type);

  return uniform;
}

}  // namespace

// TODO(donosoc): This should be loaded from the assets.
bool ParseSubShader(const std::string& source, SubShaderParseResult* out) {
  std::map<std::string, UniformBufferObject> ubo_map;
  auto lines = SplitToLines(source);
  for (auto it = lines.begin(); it != lines.end(); it++) {
    auto& line = *it;
    if (!BeginsWith(line, "//#"))
      continue;

    // We found a declaration.
    auto elements = SplitToLines(line.substr(3), ":");
    if (elements.size() != 3u) {
      LOG(ERROR, "Wrong uniform line: %s", line.c_str());
      return false;
    }

    auto uniform_opt = ParseUniform(elements);
    if (!uniform_opt)
      return false;

    auto& ubo = ubo_map[elements[0]];
    ubo.uniforms.push_back(std::move(*uniform_opt));
  }


  std::vector<UniformBufferObject> ubos;
  ubos.reserve(ubos.size());
  for (auto& [ubo_name, ubo] : ubo_map) {
    ubo.name = ubo_name;
    ubos.emplace_back(std::move(ubo));
  }

  for (auto& ubo : ubos) {
    if (!CalculateUBOLayout(&ubo)) {
      LOG(ERROR, "Could not calculate UBO layout for %s", ubo.name.c_str());
      return false;
    }
  }

  out->ubos = std::move(ubos);

  return true;
}

// CalculateUniformLayout ------------------------------------------------------

namespace {

struct UniformLayout {
  uint32_t size = 0;
  uint32_t alignment = 0;
};

uint32_t NextMultiple(uint32_t val, uint32_t multiple) {
  if (multiple == 0)
    return val;

  uint32_t remainder = val % multiple;
  if (remainder == 0)
    return val;
  return val + multiple - remainder;
}

}  // namespace

bool CalculateUBOLayout(UniformBufferObject* ubo) {
  uint32_t current_offset = 0;
  for (Uniform& uniform : ubo->uniforms) {
    if (!Valid(&uniform)) {
      LOG(ERROR, "Uniform %s is not valid.", uniform.name.c_str());
      return false;
    }

    uniform.size = GetSize(uniform.type);
    uniform.alignment = GetAlignment(uniform.type);

    uint32_t next_start = NextMultiple(current_offset, uniform.alignment);
    uniform.offset = next_start;
    current_offset = uniform.offset + uniform.size;
  }

  return true;
}

// Uniforms Utils --------------------------------------------------------------

uint32_t GetSize(UniformType type) {
  switch (type) {
    case UniformType::kBool: return 4;
    case UniformType::kInt: return 4;
    case UniformType::kFloat: return 4;
    case UniformType::kInt2: return 8;
    case UniformType::kInt3: return 12;
    case UniformType::kInt4: return 16;
    case UniformType::kUint2: return 8;
    case UniformType::kUint3: return 12;
    case UniformType::kUint4: return 16;
    case UniformType::kVec2: return 8;
    case UniformType::kVec3: return 12;
    case UniformType::kVec4: return 16;
    case UniformType::kIvec2: return 8;
    case UniformType::kIvec3: return 12;
    case UniformType::kIvec4: return 16;
    case UniformType::kUvec2: return 8;
    case UniformType::kUvec3: return 12;
    case UniformType::kUvec4: return 16;
    case UniformType::kMat4: return 64;
    case UniformType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}

uint32_t GetAlignment(UniformType type) {
  switch (type) {
    case UniformType::kBool: return 4;
    case UniformType::kInt: return 4;
    case UniformType::kFloat: return 4;
    case UniformType::kInt2: return 8;
    case UniformType::kInt3: return 16;
    case UniformType::kInt4: return 16;
    case UniformType::kUint2: return 8;
    case UniformType::kUint3: return 16;
    case UniformType::kUint4: return 16;
    case UniformType::kVec2: return 8;
    case UniformType::kVec3: return 16;
    case UniformType::kVec4: return 16;
    case UniformType::kIvec2: return 8;
    case UniformType::kIvec3: return 16;
    case UniformType::kIvec4: return 16;
    case UniformType::kUvec2: return 8;
    case UniformType::kUvec3: return 16;
    case UniformType::kUvec4: return 16;
    case UniformType::kMat4: return 16;
    case UniformType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}

UniformType FromString(const std::string& type_str) {
  UniformType type = UniformType::kLast;
  if (type_str == "bool") { type = UniformType::kBool; }
  else if (type_str == "int") { type = UniformType::kInt; }
  else if (type_str == "float") { type = UniformType::kFloat; }
  else if (type_str == "int2") { type = UniformType::kInt2; }
  else if (type_str == "int3") { type = UniformType::kInt3; }
  else if (type_str == "int4") { type = UniformType::kInt4; }
  else if (type_str == "uint2") { type = UniformType::kUint2; }
  else if (type_str == "uint3") { type = UniformType::kUint3; }
  else if (type_str == "uint4") { type = UniformType::kUint4; }
  else if (type_str == "vec2") { type = UniformType::kVec2; }
  else if (type_str == "vec3") { type = UniformType::kVec3; }
  else if (type_str == "vec4") { type = UniformType::kVec4; }
  else if (type_str == "ivec2") { type = UniformType::kIvec2; }
  else if (type_str == "ivec3") { type = UniformType::kIvec3; }
  else if (type_str == "ivec4") { type = UniformType::kIvec4; }
  else if (type_str == "uvec2") { type = UniformType::kUvec2; }
  else if (type_str == "uvec3") { type = UniformType::kUvec3; }
  else if (type_str == "uvec4") { type = UniformType::kUvec4; }
  else if (type_str == "mat4") { type = UniformType::kMat4; }

  if (type != UniformType::kLast)
    return type;

  NOT_REACHED_MSG("Got wrong type: %s", type_str.c_str());
  return type;
}

const char* ToString(UniformType type) {
  switch (type) {
    case UniformType::kBool: return "Bool";
    case UniformType::kInt: return "Int";
    case UniformType::kFloat: return "Float";
    case UniformType::kInt2: return "Int2";
    case UniformType::kInt3: return "Int3";
    case UniformType::kInt4: return "Int4";
    case UniformType::kUint2: return "Uint2";
    case UniformType::kUint3: return "Uint3";
    case UniformType::kUint4: return "Uint4";
    case UniformType::kVec2: return "Vec2";
    case UniformType::kVec3: return "Vec3";
    case UniformType::kVec4: return "Vec4";
    case UniformType::kIvec2: return "Ivec2";
    case UniformType::kIvec3: return "Ivec3";
    case UniformType::kIvec4: return "Ivec4";
    case UniformType::kUvec2: return "Uvec2";
    case UniformType::kUvec3: return "Uvec3";
    case UniformType::kUvec4: return "Uvec4";
    case UniformType::kMat4: return "Mat4";
    case UniformType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

}  // namespace rothko
