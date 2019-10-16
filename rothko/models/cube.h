// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"
#include "rothko/math/math.h"

namespace rothko {

Mesh CreateCubeMesh(VertexType, const std::string& name, Vec3 extents = {1, 1, 1});

}  // namespace
