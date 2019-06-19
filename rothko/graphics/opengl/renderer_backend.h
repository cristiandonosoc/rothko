// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/common/renderer_backend.h"

#include <string>
#include <map>

#include "rothko/utils/macros.h"

namespace rothko {

struct Shader;
struct Window;

namespace opengl {

struct MeshHandles {
  uint32_t vbo = 0;
  uint32_t ebo = 0;
  uint32_t vao = 0;
};

struct OpenGLRendererBackend : public RendererBackend {
  ~OpenGLRendererBackend();

  std::map<uint32_t, MeshHandles> loaded_meshes;

  Window* window = nullptr;
  bool loaded = false;

  // Virtual Interface ---------------------------------------------------------

  bool Init(Renderer*, InitRendererConfig*) override;

  void StartFrame() override;
  void EndFrame() override;

  // Meshes.
  bool StageMesh(Mesh*) override;
  void UnstageMesh(Mesh*) override;

  // Shaders.
  bool ParseShader(const std::string& vert_path, const std::string& frag_path,
                   Shader* out) override;
  void UnstageShader(Shader*) override;

  // Textures.
  bool StageTexture(Texture*) override;
  void UnstageTexture(Texture*) override;
};

bool Valid(OpenGLRendererBackend*);

}  // namespace opengl
}  // namespace rothko
