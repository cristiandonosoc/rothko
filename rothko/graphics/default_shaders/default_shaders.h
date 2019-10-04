// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <optional>

#include "rothko/graphics/shader.h"
#include "rothko/graphics/vertices.h"

namespace rothko {

// TODO(Cristian): Move to a function per default vertex. Otherwise knowing the UBO is "magic".
Shader CreateDefaultShader(VertexType);

}  // namespace rothko
