// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/texture.h"

#include <third_party/stb/stb_image.h>

#include "rothko/graphics/common/renderer.h"
#include "rothko/logging/logging.h"

namespace rothko {

Texture::~Texture() {
  if (Staged(this))
    RendererUnstageTexture(this->renderer, this);
  UnloadTexture(this);
}

// LoadTexture -----------------------------------------------------------------

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

  Texture tmp;
  int channels;
  tmp.data = stbi_load(path.c_str(), &tmp.dims.x, &tmp.dims.y, &channels,
                       TextureTypeToChannels(texture_type));
  if (!tmp.data.value) {
    LOG(ERROR, "Could not load texture in %s: %s",
                path.c_str(), stbi_failure_reason());
    return false;
  }

  *out= std::move(tmp);
  out->free_function = stbi_image_free;
  return true;
}

// Unload Texture ----------------------------------------------------------------------------------

void UnloadTexture(Texture* texture) {
  if (texture->data.has_value() && texture->free_function) {
    texture->free_function(texture->data.value);
    texture->data.clear();
  }
}

}  // namespace rothko
