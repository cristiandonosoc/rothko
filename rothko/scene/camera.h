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

  static OrbitCamera FromLookAt(Vec3 pos, Vec3 target);

  Vec3 target;
  Vec2 angles;      // In radians. X = pitch, Y = yaw.
  float distance;

  Vec3 dir_;        // Calculated on update.
  Vec3 pos_;        // Calculated on update.
};

void Update(OrbitCamera*);

Mat4 GetView(const OrbitCamera&);

}  // namespace rothko
