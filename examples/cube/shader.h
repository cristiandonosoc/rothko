// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/math/math.h>
#include <rothko/graphics/shader.h>

#include <memory>

struct UBO {
  ::rothko::Mat4 proj;
  ::rothko::Mat4 view;
  ::rothko::Mat4 model;
};

std::unique_ptr<::rothko::Shader> CreateShader(::rothko::Renderer*);
