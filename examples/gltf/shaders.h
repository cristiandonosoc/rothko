// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/shader.h>

namespace rothko {
namespace gltf {

std::unique_ptr<Shader> CreateNormalShader(Renderer*);

}  // namespace gltf
}  // namespace
