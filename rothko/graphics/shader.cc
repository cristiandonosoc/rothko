// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/shader.h"

#include <map>
#include <optional>

#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"
#include "rothko/utils/strings.h"
#include "rothko/utils/file.h"

namespace rothko {

Shader::~Shader() {
  if (Staged(*this))
    RendererUnstageShader(this->renderer, this);
}

bool LoadShaderSources(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out) {
  if (!ReadWholeFile(vert_path, &out->vert_src) ||
      !ReadWholeFile(frag_path, &out->frag_src)) {
    return false;
  }
  return true;
}

void RemoveSources(Shader* shader) {
  shader->vert_src.clear();
  shader->frag_src.clear();
}


// CreateVertSource --------------------------------------------------------------------------------

namespace {
  // This will be appended if |header| is null.
  constexpr char kVertexHeader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
  )";

  // This will appended always.
  constexpr char kVertexData[] = R"(
uniform vec3 camera_pos;
uniform mat4 camera_proj;
uniform mat4 camera_view;
  )";

}  // namespace

std::string CreateVertexSource(const std::string& vert_src, const char* header) {
  if (!header)
    header = kVertexHeader;
  auto src = StringPrintf("%s\n\n%s\n\n%s", header, kVertexData, vert_src.c_str());
  return src;
}

// CreateFragSource --------------------------------------------------------------------------------

namespace {
  // This will be appended if |header| is null.
  constexpr char kFragmentHeader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable
  )";

  // This will appended always.
  constexpr char kFragmentData[] = R"(
uniform vec3 camera_pos;
uniform mat4 camera_proj;
uniform mat4 camera_view;
  )";
}  // namespace

std::string CreateFragmentSource(const std::string& frag_src, const char* header) {
  if (!header)
    header = kFragmentHeader;
  auto src = StringPrintf("%s\n\n%s\n\n%s", header, kFragmentData, frag_src.c_str());
  return src;
}

}  // namespace rothko
