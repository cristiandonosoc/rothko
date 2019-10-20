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

struct OpenGLRendererBackend {
  std::map<uint32_t, MeshHandles> loaded_meshes;
  std::map<uint32_t, ShaderHandles> loaded_shaders;
  std::map<uint32_t, TextureHandles> loaded_textures;

  CameraData cameras[8] = {};
  int camera_index = -1;
};

inline const CameraData& GetCamera(const OpenGLRendererBackend& opengl) {
  return opengl.cameras[opengl.camera_index];
}

OpenGLRendererBackend* GetOpenGL();

}  // namespace opengl
}  // namespace rothko
