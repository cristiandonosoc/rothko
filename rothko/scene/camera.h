// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/vec.h"

#include "rothko/math/vec.h"

namespace rothko {

struct Camera {
  Mat4 projection;
  Mat4 view;
};

}  // namespace rothko
