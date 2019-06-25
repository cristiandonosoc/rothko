// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/containers/vector.h"

namespace rothko {

struct RenderCommand;

namespace opengl {

struct OpenGLRendererBackend;

void OpenGLExecuteCommands(const PerFrameVector<RenderCommand>& commands,
                           OpenGLRendererBackend*);

}  // namespace opengl
}  // namespace rothko
