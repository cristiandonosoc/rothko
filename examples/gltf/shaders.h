// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/shader.h>

#include <memory>

namespace rothko {
namespace gltf {

struct ModelUBO {
  /* struct Model { */
  /*   Mat4 transform; */
  /* } model; */

  /* struct Node { */
  /*   Mat4 transform; */
  /* } node; */


  struct Model {
    Mat4 transform;
    Mat4 inverse_transform;
  } model_transform;

  struct Frag {
    Vec4 base_color;
  };
};

std::unique_ptr<Shader> CreateModelShader(Renderer*);

}  // namespace gltf
}  // namespace
