// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/texture.h"

#include <third_party/stb/stb_image.h>

#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"
#include "rothko/platform/platform.h"

namespace rothko {

// Texture -----------------------------------------------------------------------------------------

Texture::~Texture() {
  if (Staged(this))
    RendererUnstageTexture(this->renderer, this);
  data.reset();
}

// LoadTexture -------------------------------------------------------------------------------------

namespace  {

int TextureTypeToChannels(TextureType type) {
  switch (type) {
    case TextureType::kRGBA: return 4;
    case TextureType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}

}  // namespace

bool STBLoadTexture(const std::string& path, TextureType texture_type, Texture* out) {
  // OpenGL expects the Y axis to be inverted.
  // TODO(Cristian): Support other rendering backends.
  stbi_set_flip_vertically_on_load(true);

  Texture tmp = {};
  tmp.name = GetBasename(path);
  tmp.type = texture_type;
  int channels;


  void* data = stbi_load(path.c_str(), &tmp.size.x, &tmp.size.y, &channels,
                         TextureTypeToChannels(texture_type));
  if (!data) {
    ERROR(Graphics, "Could not load texture in %s: %s", path.c_str(), stbi_failure_reason());
    return false;
  }


  uint32_t data_size = DataSize(tmp);
  tmp.data = std::make_unique<uint8_t[]>(data_size);
  memcpy(tmp.data.get(), data, data_size);

  *out = std::move(tmp);

  // Free the STB data.
  stbi_image_free(data);
  return true;
}

// Extras ------------------------------------------------------------------------------------------

const char* ToString(TextureType type) {
  switch (type) {
    case TextureType::kRGBA: return "RGBA";
    case TextureType::kLast: return "<last>";
  }

  NOT_REACHED_MSG("Invalid TextureType: %u", (uint32_t)type);
  return "<unknown>";
}

const char* ToString(TextureWrapMode mode) {
  switch (mode) {
    case TextureWrapMode::kClampToBorder: return "ClampToBorder";
    case TextureWrapMode::kClampToEdge: return "ClampToEdge";
    case TextureWrapMode::kMirroredRepeat: return "MirroredRepeat";
    case TextureWrapMode::kRepeat: return "Repeat";
  }

  NOT_REACHED_MSG("Invalid TextureWrapMode: %u", (uint32_t)mode);
  return "<unknown>";
}

const char* ToString(TextureFilterMode mode) {
  switch (mode) {
    case TextureFilterMode::kLinear: return "Linear";
    case TextureFilterMode::kLinearMipmapNearest: return "LinearMipmapNearest";
    case TextureFilterMode::kLinearMipampLinear: return "LinearMipampLinear";
    case TextureFilterMode::kNearest: return "Nearest";
    case TextureFilterMode::kNearestMipmapNearest: return "NearestMipmapNearest";
    case TextureFilterMode::kNearestMipmapLinear: return "NearestMipmapLinear";
  }

  NOT_REACHED_MSG("Invalid TextureFilterMode: %u", (uint32_t)mode);
  return "<unknown>";
}

uint32_t ToSize(TextureType type) {
  switch (type) {
    case TextureType::kRGBA: return 4;
    case TextureType::kLast: return 0;
  }

  NOT_REACHED();
  return 0;
}

}  // namespace rothko
