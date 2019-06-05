// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/common/renderer_backend.h"

#include <string>

#include "rothko/utils/macros.h"

namespace rothko {

struct Shader;

namespace opengl {


struct OpenGLRendererBackend : public RendererBackend {
  ~OpenGLRendererBackend();


  bool loaded = false;

  // Virtual Interface ---------------------------------------------------------

  bool Init(Renderer*, InitRendererConfig*) override;

  void StartFrame() override;
  void EndFrame() override;

  // Shader.

  bool ParseShader(const std::string& vert_path, const std::string& frag_path,
                   Shader* out) override;
  void UnstageShader(Shader*) override;
};

bool Valid(OpenGLRendererBackend*);

}  // namespace opengl
}  // namespace rothko
