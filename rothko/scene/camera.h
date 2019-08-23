// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/math.h"

#include "rothko/logging/logging.h"

namespace rothko {

enum class ProjectionType {
  kProjection,
  kOrthographic,
};

struct OrbitCamera {
  // |fov| is in radians.
  static OrbitCamera FromLookAt(Vec3 pos, Vec3 target, float fov, float aspect_ratio);

  Vec3 target;
  Vec2 angles;      // In radians. X = pitch, Y = yaw.
  float distance;

  // The percentage of |distance| that will change.
  float zoom_speed = 0.1f;

  float fov = 0;
  float aspect_ratio = 0;
  float near = 0.1f;
  float far = 100.0f;

  // Used to transform from perspective to orthographic.
  float size_per_depth_fix = 1.163f;  // Obtained experimentally.

  ProjectionType projection_type = ProjectionType::kProjection;

  Vec3 dir_;  // Calculated on update.
  Vec3 pos_;  // Calculated on update.
};

void Update(OrbitCamera*);

Mat4 GetView(const OrbitCamera& camera);

// Projection matrices.
// Both require |fov| and |aspect_ratio| to be set.


Mat4 GetPerspective(const OrbitCamera&);
Mat4 GetOrtho(const OrbitCamera&);

inline Mat4 GetProjection(const OrbitCamera& camera) {
  switch (camera.projection_type) {
    case ProjectionType::kProjection: return GetPerspective(camera);
    case ProjectionType::kOrthographic: return GetOrtho(camera);
  }

  NOT_REACHED();
  return Mat4::Identity();
}

}  // namespace rothko
