// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "rothko/graphics/common/shader.h"

namespace rothko {

struct Shader;

namespace opengl {

// These is all the information OpenGL needs to keep track of a loaded shader
// within the driver/GPU.
struct OpenGLShader {
  // These are the buffer/program handles.
  uint32_t program_handle = 0;
  uint32_t vert_ubo_handle = 0;
  uint32_t frag_ubo_handle = 0;
};

bool OpenGLParseShader(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out);

}  // namespace opengl
}  // namespace rothko
