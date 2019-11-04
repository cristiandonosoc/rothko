// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/graphics.h>

#include <rothko/utils/macros.h>

namespace rothko {
namespace simple_lighting {

#define FLOAT_PAD() float STRINGIFY(__pad_, __LINE__)

// Object Shader -----------------------------------------------------------------------------------

struct ObjectShaderUBO {
  struct Vert {
    Mat4 model = Mat4::Identity();

    // Normal matrix, that permits to correctly transform the normal to world space, taking into
    // account distorting effects such as non-uniform scaling.
    // Calculated as:
    //
    // Transpose(Inverse(model));
    Mat4 normal_matrix = Mat4::Identity();
  } vert;
  static_assert(sizeof(Vert) == 128);

  // Due to std140 alignment, Vec3 are aligned to 4 floats, thus we need a padding.
  // clang-format off
  struct Frag {
    struct Light {
      Vec4 pos;
      Vec3 ambient;       FLOAT_PAD();
      Vec3 diffuse;       FLOAT_PAD();
      Vec3 specular;

      // Point light constants.
      float constant = 1.0f;
      float linear = 0.09f;
      float quadratic = 0.032f;
    } light;

    FLOAT_PAD();
    FLOAT_PAD();

    struct Material {
      // Associated sampelr.
      Vec3 specular;
      float shininess;
    } material;

  } frag;
  // clang format on
};

Shader CreateObjectShader(Renderer*);

}  // namespace simple_lighting
}  // namespace rothko
