// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/transform.h"

#include "rothko/logging/logging.h"

namespace rothko {

// Transform ---------------------------------------------------------------------------------------

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

void TransformMatrixToTransform(const Mat4& m, Transform* transform) {
  DecomposeTransformMatrix(m, &transform->position, &transform->rotation, &transform->scale);
}

}  // namespace rothko
