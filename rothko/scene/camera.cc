// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/camera.h"

#include "rothko/logging/logging.h"

namespace rothko {

OrbitCamera OrbitCamera::FromLookAt(Vec3 pos, Vec3 target) {
  OrbitCamera camera;
  camera.target = target;
  camera.dir_ = Normalize(pos - target);
  camera.distance = Length(pos - target);

  camera.angles = EulerFromDirection(camera.dir_);

  return camera;
}

void Update(OrbitCamera* camera) {
  camera->dir_ = DirectionFromEuler(camera->angles.x, camera->angles.y);
  camera->pos_ = camera->target + camera->dir_ * camera->distance;
}

Mat4 GetView(const OrbitCamera& camera) {
  return LookAt(camera.pos_, camera.target);
}

}  // namespace rothko
