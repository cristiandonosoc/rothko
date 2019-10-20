// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/camera.h"

#include "rothko/graphics/commands.h"
#include "rothko/input/input.h"
#include "rothko/logging/logging.h"

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
    case ProjectionType::kLast: NOT_REACHED(); return Mat4::Identity();
  }

  NOT_REACHED();
  return Mat4::Identity();
}

PushCamera GetPushCamera(const OrbitCamera& camera, ProjectionType proj_override) {
  PushCamera push_camera;
  push_camera.camera_pos = camera.pos_;
  push_camera.view = GetView(camera);
  ProjectionType projection_type = camera.projection_type;

  if (proj_override != ProjectionType::kLast)
    projection_type = proj_override;
  if (projection_type == ProjectionType::kProjection) {
    push_camera.projection = GetProjection(camera);
  } else if (projection_type == ProjectionType::kOrthographic) {
    push_camera.projection = GetOrtho(camera);
  } else {
    NOT_REACHED();
  }

  return push_camera;
}

static constexpr float kMouseSensibility = 0.007f;
static float kMaxPitch = ToRadians(89.0f);

void DefaultUpdateOrbitCamera(const Input& input, OrbitCamera* camera) {
  if (input.mouse.right) {
    if (!IsZero(input.mouse_offset)) {
      camera->angles.x -= input.mouse_offset.y * kMouseSensibility;
      if (camera->angles.x > kMaxPitch) {
        camera->angles.x = kMaxPitch;
      } else if (camera->angles.x < -kMaxPitch) {
        camera->angles.x = -kMaxPitch;
      }

      camera->angles.y += input.mouse_offset.x * kMouseSensibility;
      if (camera->angles.y > kRadians360) {
        camera->angles.y -= kRadians360;
      } else if (camera->angles.y < 0) {
        camera->angles.y += kRadians360;
      }
    }
  }

  // Zoom.
  if (input.mouse.wheel.y != 0) {
    // We actually want to advance a percentage of the distance.
    camera->distance -= input.mouse.wheel.y * camera->distance * camera->zoom_speed;
    if (camera->distance < 0.5f)
      camera->distance = 0.5f;
  }

  Update(camera);
}

}  // namespace rothko
