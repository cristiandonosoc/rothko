// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/graphics/shader.h>
#include <rothko/math/math.h>

#include <memory>

namespace rothko {
namespace emulator {

struct NormalUBO {
  Mat4 proj;
  Mat4 view;
};

std::unique_ptr<Shader> CreateNormalShader(Renderer*);

}  // namespace emulator
}  // namespace rothko

