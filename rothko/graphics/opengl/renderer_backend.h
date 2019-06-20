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


struct ShaderHandles {
  uint32_t program_handle;
};

struct TextureHandles {
  uint32_t tex_handle = 0;
};

struct OpenGLRendererBackend : public RendererBackend {
  OpenGLRendererBackend();
  ~OpenGLRendererBackend();
  DELETE_COPY_AND_ASSIGN(OpenGLRendererBackend);
  DELETE_MOVE_AND_ASSIGN(OpenGLRendererBackend);

  std::map<uint32_t, MeshHandles> loaded_meshes;
  std::map<uint32_t, ShaderHandles> loaded_shaders;
  std::map<uint32_t, TextureHandles> loaded_textures;

  Window* window = nullptr;

  // Virtual Interface ---------------------------------------------------------

  bool Init(Renderer*, InitRendererConfig*) override;

  void StartFrame() override;
  void EndFrame() override;

  // Meshes.
  bool StageMesh(Mesh*) override;
  void UnstageMesh(Mesh*) override;

  // Shaders.
  bool StageShader(Shader*) override;
  void UnstageShader(Shader*) override;

  // Textures.
  bool StageTexture(const StageTextureConfig&, Texture*) override;
  void UnstageTexture(Texture*) override;
};

inline bool Valid(const OpenGLRendererBackend& o) { return !!o.window; }

}  // namespace opengl
}  // namespace rothko
