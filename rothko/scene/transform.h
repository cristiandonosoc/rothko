// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/math/math.h"
#include "rothko/utils/types.h"

#include <bitset>
#include <vector>

namespace rothko {

struct SceneGraph;

// Transform ---------------------------------------------------------------------------------------

#pragma pack(push, 1)
struct Transform {
  static constexpr uint32_t kInvalidIndex = (uint32_t)-1;

  Vec3 position;
  Vec3 rotation;
  Vec3 scale;

  // Positions within the scene graph storage.
  uint32_t index = kInvalidIndex;
  uint32_t parent = kInvalidIndex;
  std::vector<uint32_t> children;

  // Represents the overal transformation matrix. Recreated by |Update|.
  Mat4 world_matrix_;
};
#pragma pack(pop)
// position + rotation + scale + index + parent + children + world_matrix_.
static_assert(sizeof(Transform) == 132);

// Calculates the transformation matrix from |position|, |rotation| and |scale|.
Mat4 GetLocalTransformMatrix(const Transform&);

// Calculates the overall world transformation. |LocalTransform| * |parent's world transform|.
// Assumes |parent.world_matrix_| is updated.
Mat4 GetWorlTransformMatrix(const SceneGraph&, const Transform&);

// Calculates |matrix_| from the transform data.
void Update(const SceneGraph&, Transform*);

inline const Mat4& GetWorldTransformationMatrix(const Transform& t) { return t.world_matrix_; }

// SceneGraph -------------------------------------------------------------------------------------

constexpr uint64_t kSceneGraphSize = 8192;

struct SceneGraph {
  Transform transforms[kSceneGraphSize];
  std::bitset<kSceneGraphSize> used;
  std::bitset<kSceneGraphSize> dirty;

  uint64_t count = 0;
};
static_assert(sizeof(SceneGraph) == KILOBYTES(1056) + KILOBYTES(2) + 8);

// Gives a cleared transform. Comes with the correct |index| set.
Transform* AddTransform(SceneGraph*);

void DeleteTransform(SceneGraph*, uint32_t index);

inline void DeleteTransform(SceneGraph* scene_graph, Transform* transform) {
  DeleteTransform(scene_graph, transform->index);
}

}  // namespace rothko
