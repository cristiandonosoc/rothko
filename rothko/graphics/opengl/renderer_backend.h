// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/common/renderer_backend.h"

#include "rothko/utils/macros.h"

namespace rothko {
namespace opengl {


struct OpenGLRendererBackend : public RendererBackend {
  ~OpenGLRendererBackend();


  bool loaded = false;

  // Virtual Interface ---------------------------------------------------------

  bool Init(Renderer*, InitRendererConfig*) override;

  void StartFrame() override;
  void EndFrame() override;
};

bool Valid(OpenGLRendererBackend*);

}  // namespace opengl
}  // namespace rothko
