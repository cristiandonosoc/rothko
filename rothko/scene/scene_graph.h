// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/scene/transform.h"

namespace rothko {

// SceneNode ---------------------------------------------------------------------------------------

#pragma pack(push, 1)
struct SceneNode {
  static constexpr uint32_t kInvalidIndex = (uint32_t)-1;

  uint32_t index = kInvalidIndex;
  uint32_t parent = kInvalidIndex;

  static constexpr uint32_t kDirtyFlag = (1 << 0);

  std::vector<uint32_t> children;

  Transform transform;
};
#pragma pack(pop)
static_assert(sizeof(SceneNode) == 132);

// Calculates the overall world transformation. |LocalTransform| * |parent's world transform|.
// Assumes |parent.world_matrix_| is updated.
// If |scene_graph| is null or if |parent| is not set, will return the local transform.
Mat4 GetWorlTransformMatrix(const SceneNode&, const SceneNode* parent);

void Update(SceneNode* node, const SceneNode* parent = nullptr);

// SceneGraph --------------------------------------------------------------------------------------

constexpr uint64_t kSceneGraphSize = 8192;

struct SceneGraph {
  SceneNode nodes[kSceneGraphSize];

  std::bitset<kSceneGraphSize> used;

  uint64_t count = 0;
};

// Gives a cleared transform. Comes with the correct |index| set.
SceneNode* AddNode(SceneGraph*, uint32_t parent_index = SceneNode::kInvalidIndex);

inline SceneNode* AddTransform(SceneGraph* scene_graph, SceneNode* parent) {
  return AddNode(scene_graph, parent->index);
}

// Deletes all the child (recursive) transform. Updates the parent (if set).
void DeleteTransform(SceneGraph*, uint32_t index, uint32_t parent_transform);

inline void DeleteNode(SceneGraph* scene_graph, SceneNode* node) {
  DeleteTransform(scene_graph, node->index, node->parent);
}

// Goes over all the transforms and calculates the new transforms.
// NOTE: This assumes that node 0 is the root.
// NOTE: If this becomes slow, tracking of when transforms changes to only update dirty
//       transformations can be done.
void Update(SceneGraph*);

}  // namespace rothko
