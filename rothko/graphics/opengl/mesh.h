// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace rothko {

struct Mesh;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageMesh(OpenGLRendererBackend*, Mesh*);
void OpenGLUnstageMesh(OpenGLRendererBackend*, Mesh*);

}  // namespace opengl
}  // namespace rothko
