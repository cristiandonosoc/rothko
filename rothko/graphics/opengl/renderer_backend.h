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

struct OpenGLRendererBackend {
  Vec3 camera_pos;
  Mat4 camera_projection = Mat4::Identity();
  Mat4 camera_view = Mat4::Identity();

  std::map<uint32_t, MeshHandles> loaded_meshes;
  std::map<uint32_t, ShaderHandles> loaded_shaders;
  std::map<uint32_t, TextureHandles> loaded_textures;
};

OpenGLRendererBackend* GetOpenGL();

}  // namespace opengl
}  // namespace rothko
