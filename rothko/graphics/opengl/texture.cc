// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/texture.h"

#include <GL/gl3w.h>

#include <atomic>

#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/graphics/renderer.h"
#include "rothko/graphics/texture.h"
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

GLenum TextureTypeToGL(TextureType type) {
  switch (type) {
    case TextureType::kRGBA: return GL_RGBA;
    case TextureType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}

// Stage Texture -----------------------------------------------------------------------------------

GLenum WrapToGL(TextureWrapMode wrap) {
  switch (wrap) {
    case TextureWrapMode::kClampToBorder: return GL_CLAMP_TO_BORDER;
    case TextureWrapMode::kClampToEdge: return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::kMirroredRepeat: return GL_MIRRORED_REPEAT;
    case TextureWrapMode::kRepeat: return GL_REPEAT;
    case TextureWrapMode::kLast: return GL_NONE;
  }

  NOT_REACHED();
  return 0;
}

GLenum FilterToGL(TextureFilterMode filter) {
  switch (filter) {
    case TextureFilterMode::kNearest: return GL_NEAREST;
    case TextureFilterMode::kLinear: return GL_LINEAR;
    case TextureFilterMode::kNearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
    case TextureFilterMode::kNearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
    case TextureFilterMode::kLinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
    case TextureFilterMode::kLinearMipampLinear: return GL_LINEAR_MIPMAP_LINEAR;
    case TextureFilterMode::kLast: return GL_NONE;
  }

  NOT_REACHED();
  return 0;
}

}  // namespace

bool OpenGLStageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  LOG(OpenGL, "Staging texture %s [Type: %s, Size: %s]",
              texture->name.c_str(),
              ToString(texture->type),
              ToString(texture->size).c_str());

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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(texture->wrap_mode_u));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(texture->wrap_mode_v));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterToGL(texture->min_filter));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterToGL(texture->mag_filter));

  // Send the bits over.
  glTexImage2D(GL_TEXTURE_2D,         // target
               0,                     // level
               GL_RGBA8,              // internalformat
               texture->size.width,   // width,
               texture->size.height,  // height
               0,                     // border
               GL_RGBA,               // format
               GL_UNSIGNED_BYTE,      // type,
               texture->data.get());

  if (texture->mipmaps)
    glGenerateMipmap(GL_TEXTURE_2D);

  TextureHandles handles;
  handles.tex_handle = handle;
  opengl->loaded_textures[uuid] = std::move(handles);

  glBindTexture(GL_TEXTURE_2D, NULL);
  texture->uuid = uuid;

  return true;
}

// Unstage Texture ---------------------------------------------------------------------------------

void OpenGLUnstageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid.value);
  ASSERT(it != opengl->loaded_textures.end());

  glDeleteTextures(1, &it->second.tex_handle);
  opengl->loaded_textures.erase(it);
  texture->uuid = 0;
}

// Sub Tex -----------------------------------------------------------------------------------------

void OpenGLSubTexture(OpenGLRendererBackend* opengl, Texture* texture, void* data,
                      Int2 offset, Int2 range) {
  ASSERT(offset.x + range.x <= texture->size.x);
  ASSERT(offset.y + range.y >= 0 && range.y <= texture->size.y);

  if (IsZero(offset) && IsZero(range))
    range = texture->size;

  if (data == nullptr)
    data = texture->data.get();

  auto it = opengl->loaded_textures.find(texture->uuid.value);
  ASSERT(it != opengl->loaded_textures.end());

  glBindTexture(GL_TEXTURE_2D, it->second.tex_handle);
  glTexSubImage2D(GL_TEXTURE_2D,
                  0,
                  offset.x,
                  offset.y,
                  range.width,
                  range.height,
                  TextureTypeToGL(texture->type),
                  GL_UNSIGNED_BYTE,
                  data);
}

}  // namespace opengl
}  // namespace rothko
