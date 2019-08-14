// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/texture.h"

namespace rothko {

struct StageTextureConfig;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageTexture(OpenGLRendererBackend*, Texture*, const StageTextureConfig&);
void OpenGLUnstageTexture(OpenGLRendererBackend*, Texture*);
void OpenGLSubTexture(OpenGLRendererBackend*, Texture*, Int2 offset, Int2 range, void* data);

}  // namespace opengl
}  // namespace rothko
