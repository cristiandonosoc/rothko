// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <map>
#include <string>

#include "rothko/graphics/opengl/shader.h"
#include "rothko/math/math.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Shader;
struct Texture;
struct Window;

namespace opengl {

struct MeshHandles {
  uint32_t vbo = 0;
  uint32_t ebo = 0;
  uint32_t vao = 0;
};

struct TextureHandles {
  uint32_t tex_handle = 0;
};

struct CameraData {
  Vec3 pos = {};
  Mat4 projection = Mat4::Identity();
  Mat4 view = Mat4::Identity();
};

struct Config {
  Int2 viewport_pos = {};
  Int2 viewport_size = {};
};

struct OpenGLRendererBackend {
  // NOTE: This are NON-OWNING pointers. While the backend will keep correct track of these (won't
  //       dangling), it can give these references outside of the renderer system and it's the
  //       client's responsability to correctly handle those.
  std::map<std::string, const Shader*> shader_map;


  std::map<uint32_t, MeshHandles> loaded_meshes;
  std::map<uint32_t, ShaderHandles> loaded_shaders;
  std::map<uint32_t, TextureHandles> loaded_textures;

  std::unique_ptr<Texture> white_texture;

  CameraData cameras[8] = {};
  int camera_index = -1;

  Config configs[8] = {};
  int config_index = -1;
};

inline const CameraData& GetCamera(const OpenGLRendererBackend& opengl) {
  return opengl.cameras[opengl.camera_index];
}

inline const Config& GetConfig(const OpenGLRendererBackend& opengl) {
  return opengl.configs[opengl.config_index];
}

OpenGLRendererBackend* GetOpenGL();

}  // namespace opengl
}  // namespace rothko
