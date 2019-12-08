// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/math.h"
#include "rothko/utils/types.h"

#include <bitset>
#include <vector>

namespace rothko {

#pragma pack(push, 1)
struct Transform {
  Transform(Vec3 position = {}, Vec3 rotation = {}, Vec3 scale = {1, 1, 1})
      : position(position), rotation(rotation), scale(scale), world_matrix(Mat4::Identity()) {}

  Vec3 position;
  Vec3 rotation;
  Vec3 scale;

  // Represents the overal transformation matrix. Recreated by |Update|.
  // Should not be touched directly.
  Mat4 world_matrix;

  Transform operator+(const Transform& t) const;
  void operator+=(const Transform& t);

  Transform operator-(const Transform& t) const;
  void operator-=(const Transform& t);
};
#pragma pack(pop)
static_assert(sizeof(Transform) == 100);

Mat4 CalculateTransformMatrix(const Transform&);

/* // Calculates the transformation matrix from |position|, |rotation| and |scale|. */
/* Mat4 GetLocalTransformMatrix(const Transform&); */

// Calculates |matrix_| from the transform data.
// If |scene_graph| is null or if |transform.parent| is not set, will use the local transform.
void Update(Transform*);

// NOTE: It will NOT update the transform, so only the transform vectors will be changed.
//       This is *global* decomposition, so this will make it out of sync with parent
//       transformations.
Transform TransformMatrixToTransform(const Mat4&);

inline Vec3 GetWorldPosition(const Transform& transform) {
  return PositionFromTransformMatrix(transform.world_matrix);
}

inline Vec3 GetWorldRotation(const Transform& transform) {
  return RotationFromTransformMatrix(transform.world_matrix);
}

inline Vec3 GetWorldScale(const Transform& transform) {
  return ScaleFromTransformMatrix(transform.world_matrix);
}

// Vec3{1, 0, 0} rotated by |world_matrix|.
// No translation is applied.
Vec3 GetWorldDirection(const Transform&);

std::string ToString(const Transform&);

}  // namespace rothko
