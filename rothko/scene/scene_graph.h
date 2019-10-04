// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/scene/transform.h"

namespace rothko {

constexpr uint64_t kSceneGraphSize = 8192;

struct SceneGraph {
  Transform transforms[kSceneGraphSize];
  std::bitset<kSceneGraphSize> used;
  std::bitset<kSceneGraphSize> dirty;

  uint64_t count = 0;
};
static_assert(sizeof(SceneGraph) == KILOBYTES(1056) + KILOBYTES(2) + 8);

// Gives a cleared transform. Comes with the correct |index| set.
Transform* AddTransform(SceneGraph*, uint32_t parent_index = Transform::kInvalidIndex);

inline Transform* AddTransform(SceneGraph* scene_graph, Transform* parent) {
  return AddTransform(scene_graph, parent->index);
}

// Deletes all the child (recursive) transform. Updates the parent (if set).
void DeleteTransform(SceneGraph*, uint32_t index, uint32_t parent_transform);

inline void DeleteTransform(SceneGraph* scene_graph, Transform* transform) {
  DeleteTransform(scene_graph, transform->index, transform->parent_index);
}

// Goes over all the dirty transform and updates them.
// Does two passes:
//
// 1. Detect all the transforms that should be updated. This is because if a transform is dirty, all
//    the children have to be marked as dirty as well.
// 2. Go over all the dirty elements and update them.
void Update(SceneGraph*);

inline void SetDirty(SceneGraph* scene_graph, Transform* transform) {
  scene_graph->dirty[transform->index] = 1;
}

}  // namespace rothko
