// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/math.h"

#include "rothko/logging/logging.h"

namespace rothko {

struct Input;
struct PushCamera;

enum class ProjectionType {
  kProjection,
  kOrthographic,
  kLast,
};

// OrbitCamera -------------------------------------------------------------------------------------

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

  Vec3 pos_;  // Calculated on update.
  Vec3 dir_;  // Calculated on update.
};

void Update(OrbitCamera*);

Mat4 GetView(const OrbitCamera& camera);

// Projection matrices.
// Both require |fov| and |aspect_ratio| to be set.

Mat4 GetPerspective(const OrbitCamera&);
Mat4 GetOrtho(const OrbitCamera&);

// Returns either ortho or perspective projection, depending on the camera type.
Mat4 GetProjection(const OrbitCamera&);

// If |proj_override| != ProjectionType::KLast, it will use it instead of |camera.projection_type|.
PushCamera GetPushCamera(const OrbitCamera&, ProjectionType proj_override = ProjectionType::kLast);

// Moves the target in a warped "local" frame, where the axis are:
//
// X: dir_ in XZ plane.
// Y: Up (Vec3(0, 1, 0)).
// Z: -Cross(dir_, up).
//
// Assumes the camera |dir_| is updated. Does not update the camera.
void MoveInLocalFrame(OrbitCamera*, Vec3 offset);

void DefaultUpdateOrbitCamera(const Input&, OrbitCamera*);

}  // namespace rothko
