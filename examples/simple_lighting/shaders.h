// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/graphics.h>

namespace rothko {
namespace simple_lighting {

// Object Shader -----------------------------------------------------------------------------------

struct ObjectShaderUBO {
  struct Vert {
    Mat4 model = Mat4::Identity();
  } vert;

  // Due to std140 alignment, Vec3 are aligned to 4 floats, thus we need a padding.
  // clang-format off
  struct Frag {
    Vec3 object_color;  float _pad1;
    Vec3 light_color;   float _pad2;
    Vec3 light_pos;     float _pad3;
  } frag;
  // clang format on
};

Shader CreateObjectShader(Renderer*);

// Light Shader ------------------------------------------------------------------------------------

struct LightShaderUBO {
  struct Vert {
    Mat4 model;
  } vert;

  struct Frag {
    Vec3 light_color;
  } frag;
};

Shader CreateLightShader(Renderer*);

}  // namespace simple_lighting
}  // namespace rothko
