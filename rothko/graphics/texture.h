// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>
#include <string>

#include "rothko/math/math.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Renderer;

enum class TextureType : uint8_t {
  kRGBA,  // 32 bits.
  kLast,
};
const char* ToString(TextureType);
uint32_t ToSize(TextureType type);

enum class TextureWrapMode : uint8_t {
  kClampToBorder,
  kClampToEdge,
  kMirroredRepeat,
  kRepeat,
};
const char* ToString(TextureWrapMode);

enum class TextureFilterMode : uint8_t {
  kLinear,
  kLinearMipmapNearest,
  kLinearMipampLinear,
  kNearest,
  kNearestMipmapNearest,
  kNearestMipmapLinear,
};
const char* ToString(TextureFilterMode);

struct Texture {
  RAII_CONSTRUCTORS(Texture);

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;

  std::string name;
  Int2 size;

  TextureType type = TextureType::kLast;

  TextureWrapMode wrap_mode_u = TextureWrapMode::kRepeat;
  TextureWrapMode wrap_mode_v = TextureWrapMode::kRepeat;

  TextureFilterMode min_filter = TextureFilterMode::kLinear;
  TextureFilterMode mag_filter = TextureFilterMode::kLinear;

  uint8_t mipmaps = 1;

  std::unique_ptr<uint8_t[]> data;
};

inline bool Loaded(const Texture& t) { return !!t.data; }
inline bool Staged(const Texture& t) { return t.uuid.has_value(); }

inline uint32_t DataSize(const Texture& t) { return t.size.x * t.size.y * ToSize(t.type); }

bool STBLoadTexture(const std::string& path, TextureType, Texture* out);

}  // namespace rothko
