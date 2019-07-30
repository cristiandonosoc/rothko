// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/texture.h"

#include <GL/gl3w.h>

#include <atomic>

#include "rothko/graphics/common/renderer.h"
#include "rothko/graphics/common/texture.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/logging/logging.h"

namespace rothko {
namespace opengl {

namespace {

std::atomic<uint32_t> kNextTextureUUID = 1;
uint32_t GetNextTextureUUID() {
  uint32_t id = kNextTextureUUID++;
  ASSERT(id < UINT32_MAX);
  return id;
}

// Stage Texture ---------------------------------------------------------------

GLenum WrapToGL(StageTextureConfig::Wrap wrap) {
  switch (wrap) {
    case StageTextureConfig::Wrap::kClampToEdge: return GL_CLAMP_TO_EDGE;
    case StageTextureConfig::Wrap::kMirroredRepeat: return GL_MIRRORED_REPEAT;
    case StageTextureConfig::Wrap::kRepeat: return GL_REPEAT;
  }

  NOT_REACHED();
  return 0;
}

GLenum FilterToGL(StageTextureConfig::Filter filter) {
  switch (filter) {
    case StageTextureConfig::Filter::kNearest: return GL_NEAREST;
    case StageTextureConfig::Filter::kLinear: return GL_LINEAR;
    case StageTextureConfig::Filter::kNearestMipmapNearest:
      return GL_NEAREST_MIPMAP_NEAREST;
    case StageTextureConfig::Filter::kNearestMipmapLinear:
      return GL_NEAREST_MIPMAP_LINEAR;
    case StageTextureConfig::Filter::kLinearMipmapNearest:
      return GL_LINEAR_MIPMAP_NEAREST;
    case StageTextureConfig::Filter::kLinearMipampLinear:
      return GL_LINEAR_MIPMAP_LINEAR;
  }

  NOT_REACHED();
  return 0;
}

}  // namespace

bool OpenGLStageTexture(const StageTextureConfig& config,
                        OpenGLRendererBackend* opengl,
                        Texture* texture) {
  uint32_t uuid = GetNextTextureUUID();
  auto it = opengl->loaded_textures.find(uuid);
  if (it != opengl->loaded_textures.end()) {
    ERROR(OpenGL, "Texture %s is already loaded.", texture->name.c_str());
    return false;
  }

  uint32_t handle;
  glGenTextures(1, &handle);
  glBindTexture(GL_TEXTURE_2D, handle);

  // Setup wrapping/filtering options.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  WrapToGL(config.wrap_u));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  WrapToGL(config.wrap_v));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  FilterToGL(config.min_filter));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  FilterToGL(config.max_filter));

  // Send the bits over.
  glTexImage2D(GL_TEXTURE_2D,           // target
               0,                       // level
               GL_RGBA,                 // internalformat
               texture->dims.width,     // width,
               texture->dims.height,    // height
               0,                       // border
               GL_RGBA,                 // format
               GL_UNSIGNED_BYTE,        // type,
               texture->data.value);

  if (config.generate_mipmaps)
    glGenerateMipmap(GL_TEXTURE_2D);

  TextureHandles handles;
  handles.tex_handle = handle;
  opengl->loaded_textures[uuid] = std::move(handles);

  glBindTexture(GL_TEXTURE_2D, NULL);
  texture->uuid = uuid;

  return true;
}

// Unstage Texture -------------------------------------------------------------

void OpenGLUnstageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid.value);
  ASSERT(it != opengl->loaded_textures.end());

  glDeleteTextures(1, &it->second.tex_handle);
  opengl->loaded_textures.erase(it);
  texture->uuid = 0;
}

}  // namespace opengl
}  // namespace rothko
