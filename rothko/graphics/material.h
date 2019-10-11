// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/math/math.h>

namespace rothko {

struct Texture;

struct Material {
  Texture* base_texture = nullptr;
  Vec4 base_color;
};

}  // namespace rothko
