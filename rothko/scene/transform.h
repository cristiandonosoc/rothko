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
  static constexpr uint32_t kInvalidIndex = (uint32_t)-1;

  Transform(Vec3 position = {}, Vec3 rotation = {}, Vec3 scale = {1, 1, 1})
      : position(position), rotation(rotation), scale(scale) {}

  Vec3 position;
  Vec3 rotation;
  Vec3 scale;

  // Positions within the scene graph storage.
  uint32_t index = kInvalidIndex;
  uint32_t parent_index = kInvalidIndex;
  std::vector<uint32_t> children;

  // Represents the overal transformation matrix. Recreated by |Update|.
  // Should not be touched directly.
  Mat4 world_matrix;
};

#pragma pack(pop)
// position + rotation + scale + index + parent + children + world_matrix_.
static_assert(sizeof(Transform) == 132);

// Calculates the transformation matrix from |position|, |rotation| and |scale|.
Mat4 GetLocalTransformMatrix(const Transform&);

// Calculates the overall world transformation. |LocalTransform| * |parent's world transform|.
// Assumes |parent.world_matrix_| is updated.
// If |scene_graph| is null or if |transform.parent| is not set, will return the local transform.
Mat4 GetWorlTransformMatrix(const Transform&, const Transform* parent);

// Calculates |matrix_| from the transform data.
// If |scene_graph| is null or if |transform.parent| is not set, will use the local transform.
void Update(Transform*, const Transform* parent = nullptr);


}  // namespace rothko
