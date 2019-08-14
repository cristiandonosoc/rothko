// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/containers/vector.h"

namespace rothko {

struct RenderCommand;

namespace opengl {

struct OpenGLRendererBackend;

void OpenGLExecuteCommands(OpenGLRendererBackend*, const PerFrameVector<RenderCommand>& commands);

}  // namespace opengl
}  // namespace rothko
