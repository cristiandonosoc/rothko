// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "rothko/math/math.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Renderer;

enum class TextureType : uint32_t {
  kRGBA,  // 32 bits.
  kLast,
};

struct Texture {
  RAII_CONSTRUCTORS(Texture);

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;
  TextureType type = TextureType::kLast;

  std::string name;
  Int2 dims;

  // The type of the function this texture should use to free the data upon
  // shutdown. If null, means that the data memory lifetime is handled by
  // someone else.
  using FreeFunction = void(*)(void*);
  FreeFunction free_function = nullptr;
  ClearOnMove<uint8_t*> data = nullptr;
};

inline bool Staged(Texture* t) { return t->uuid.has_value(); }

bool STBLoadTexture(const std::string& path, TextureType, Texture* out);

}  // namespace rothko
