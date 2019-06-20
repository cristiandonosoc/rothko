// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "rothko/graphics/common/shader.h"

namespace rothko {

struct Shader;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageShader(OpenGLRendererBackend*, Shader*);
void OpenGLUnstageShader(OpenGLRendererBackend*, Shader*);

}  // namespace opengl
}  // namespace rothko
