// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/transform.h"

#include <sstream>

#include "rothko/logging/logging.h"

namespace rothko {

// Transform Operations ----------------------------------------------------------------------------

Transform Transform::operator+(const Transform& t) const {
  Transform result;
  result.position = position + t.position;
  result.rotation = rotation + t.rotation;
  result.scale = scale + t.scale;
  return result;
}

void Transform::operator+=(const Transform& t) {
  position += t.position;
  rotation += t.rotation;
  scale += t.scale;
}

Transform Transform::operator-(const Transform& t) const {
  Transform result;
  result.position = position - t.position;
  result.rotation = rotation - t.rotation;
  result.scale = scale - t.scale;
  return result;
}

void Transform::operator-=(const Transform& t) {
  position -= t.position;
  rotation -= t.rotation;
  scale -= t.scale;
}

// Functions ---------------------------------------------------------------------------------------

Mat4 CalculateTransformMatrix(const Transform& transform) {
  Mat4 result = Mat4::Identity();
  result *= Translate(transform.position);
  result *= Rotate({0, 0, 1}, transform.rotation.z);
  result *= Rotate({0, 1, 0}, transform.rotation.y);
  result *= Rotate({1, 0, 0}, transform.rotation.x);
  result *= Scale(transform.scale);

  return result;
}

void Update(Transform* transform) {
  transform->world_matrix = CalculateTransformMatrix(*transform);
}

Transform TransformMatrixToTransform(const Mat4& m) {
  Transform transform = {};
  DecomposeTransformMatrix(m, &transform.position, &transform.rotation, &transform.scale);
  return transform;
}

Vec3 GetWorldDirection(const Transform& transform) {
  Mat3 m = ToMat3(transform.world_matrix);
  return m * Vec3{1, 0, 0};
}

std::string ToString(const Transform& transform) {
  std::stringstream ss;

  ss << "Position: " << ToString(transform.position) << std::endl
     << "Rotation: " << ToString(transform.rotation) << std::endl
     << "Scale:    " << ToString(transform.scale);

  return ss.str();
}

}  // namespace rothko
