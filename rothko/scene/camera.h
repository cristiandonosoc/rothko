// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/math.h"

namespace rothko {

struct Camera {
  Vec3 pos;
  Vec3 target;
};

struct OrbitCamera {
  Vec3 target;
  Vec2 angles;    // In radians.
  float distance;
};

Mat4 GetView(const OrbitCamera&);

}  // namespace rothko
