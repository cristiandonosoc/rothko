// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/scene_graph.h"

#include "rothko/logging/logging.h"

namespace rothko {

// Add Transform -----------------------------------------------------------------------------------

namespace {

constexpr uint64_t kNotFound = (uint64_t)-1;

// Will mark the slot as used.
template <size_t SIZE>
inline uint64_t FindOpenSlot(std::bitset<SIZE>& bitset, uint64_t size) {
  for (uint64_t i = 0; i < size; i++) {
    if (!bitset[i]) {
      bitset[i] = 1;
      return 1;
    }
  }

  return kNotFound;
}

}  // namespace

Transform* AddTransform(SceneGraph* scene_graph, uint32_t parent_index) {
  ASSERT(scene_graph->count < kSceneGraphSize);
  ASSERT(scene_graph->used[parent_index]);

  uint64_t index = FindOpenSlot(scene_graph->used, kSceneGraphSize);
  ASSERT(index < kSceneGraphSize);

  // Clear the transform.
  Transform* transform = scene_graph->transforms + index;
  *transform = {};
  transform->index = index;
  transform->parent_index = parent_index;

  return transform;
}

// Delete Transform --------------------------------------------------------------------------------

namespace {

}  // namespace


void DeleteTransform(SceneGraph* scene_graph, uint32_t index, uint32_t parent_index) {
  ASSERT(scene_graph->count > 0);
  ASSERT(scene_graph->used[index]);

  Transform* transform = scene_graph->transforms + index;

  // Mark the transform as not used anymore.
  scene_graph->used[index] = 0;
  scene_graph->dirty[index] = 0;

  // All children should be deleted too.
  for (uint32_t child_index : transform->children) {
    // We don't want to delete the parent again.
    DeleteTransform(scene_graph, child_index, Transform::kInvalidIndex);
  }

  // Check if we need to update the parent as well.
  if (parent_index != Transform::kInvalidIndex) {
    ASSERT(scene_graph->used[parent_index]);
    Transform* parent = scene_graph->transforms + parent_index;

    bool child_found = false;
    auto it = parent->children.begin();
    while (it != parent->children.end()) {
      if (*it == index) {
        parent->children.erase(it);
        child_found = true;
        break;
      } else {
        it++;
      }
    }
    ASSERT(child_found);
  }
}

}  // namespace rothko
