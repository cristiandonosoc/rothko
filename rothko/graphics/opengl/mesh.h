// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/math.h"

namespace rothko {

struct Mesh;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageMesh(OpenGLRendererBackend*, Mesh*);
void OpenGLUnstageMesh(OpenGLRendererBackend*, Mesh*);

bool OpenGLUploadMeshRange(OpenGLRendererBackend*, Mesh*, Int2 vertex_range, Int2 index_range);

}  // namespace opengl
}  // namespace rothko
