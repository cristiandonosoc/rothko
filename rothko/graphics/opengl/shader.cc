// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/shader.h"

#include <optional>
#include <vector>

#include "rothko/utils/file.h"

namespace rothko {
namespace opengl {

bool OpenGLParseShader(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out) {
  // Vertex shader.
  std::string vert_source;
  SubShaderParseResult vert_parse;
  if (!ReadWholeFile(vert_path, &vert_source) ||
      !ParseSubShader(vert_source, &vert_parse)) {
    return false;
  }

  // Fragment shader.
  std::string frag_source;
  SubShaderParseResult frag_parse;
  if (!ReadWholeFile(frag_path, &frag_source) ||
      !ParseSubShader(frag_source, &frag_parse)) {
    return false;
  }

  out->vert_source = std::move(vert_source);
  out->frag_source = std::move(frag_source);
  out->vert_ubos = std::move(vert_parse.ubos);
  out->frag_ubos = std::move(frag_parse.ubos);

  return true;
}

}  // namespace opengl
}  // namespace rothko

