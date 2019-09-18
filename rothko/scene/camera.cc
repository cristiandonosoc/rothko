// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/camera.h"

#include "rothko/logging/logging.h"
#include "rothko/graphics/commands.h"

namespace rothko {

OrbitCamera OrbitCamera::FromLookAt(Vec3 pos, Vec3 target, float fov, float aspect_ratio) {
  OrbitCamera camera;
  camera.target = target;
  camera.dir_ = Normalize(pos - target);
  camera.distance = Length(pos - target);

  camera.angles = EulerFromDirection(camera.dir_);

  camera.fov = fov;
  camera.aspect_ratio = aspect_ratio;

  return camera;
}

void Update(OrbitCamera* camera) {
  camera->dir_ = DirectionFromEuler(camera->angles.x, camera->angles.y);
  camera->pos_ = camera->target + camera->dir_ * camera->distance;
}

Mat4 GetView(const OrbitCamera& camera) {
  return LookAt(camera.pos_, camera.target);
}

Mat4 GetPerspective(const OrbitCamera& camera) {
  ASSERT(camera.fov > 0);
  ASSERT(camera.aspect_ratio > 0);

  return Perspective(camera.fov, camera.aspect_ratio, camera.near, camera.far);
}

Mat4 GetOrtho(const OrbitCamera& camera) {
  ASSERT(camera.fov > 0);
  ASSERT(camera.aspect_ratio > 0);

  // The amount the bounding box of the camera grows for each unit of depth (distance).
  float size_per_depth = Atan(camera.fov / 2.0f) * camera.size_per_depth_fix;
  float side_size = size_per_depth * camera.distance;

  Vec2 size = {side_size * camera.aspect_ratio, side_size};

  return Ortho(-size.x, size.x, -size.y, size.y, -10.0f, camera.far);
}

Mat4 GetProjection(const OrbitCamera& camera) {
  switch (camera.projection_type) {
    case ProjectionType::kProjection: return GetPerspective(camera);
    case ProjectionType::kOrthographic: return GetOrtho(camera);
  }

  NOT_REACHED();
  return Mat4::Identity();
}

PushCamera GetCommand(const OrbitCamera& camera) {
  PushCamera push_camera;
  push_camera.camera_pos = camera.pos_;
  push_camera.view = GetView(camera);
  push_camera.projection = GetProjection(camera);

  return push_camera;
}

}  // namespace rothko
