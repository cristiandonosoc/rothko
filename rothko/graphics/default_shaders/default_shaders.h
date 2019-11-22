// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "rothko/graphics/shader.h"
#include "rothko/graphics/vertices.h"

namespace rothko {

std::unique_ptr<Shader> CreateDefaultShader(Renderer*, VertexType);

}  // namespace rothko
