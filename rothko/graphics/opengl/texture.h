// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace rothko {

struct StageTextureConfig;
struct Texture;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageTexture(OpenGLRendererBackend*, Texture*,
                        const StageTextureConfig&);
void OpenGLUnstageTexture(OpenGLRendererBackend*, Texture*);

}  // namespace opengl
}  // namespace rothko
