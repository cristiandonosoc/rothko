// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/scene_graph.h"

#include "rothko/logging/logging.h"

namespace rothko {

// SceneNode ---------------------------------------------------------------------------------------

Mat4 GetWorlTransformMatrix(const SceneNode& node, const SceneNode* parent) {
  Mat4 local_transform = CalculateTransformMatrix(node.transform);
  if (!parent)
    return local_transform;
  return parent->transform.world_matrix * local_transform;
}

void Update(SceneNode* node, const SceneNode* parent) {
  node->transform.world_matrix = GetWorlTransformMatrix(*node, parent);
}

SceneNode* GetParent(SceneGraph* scene_graph, SceneNode* node) {
  if (node->parent_index == SceneNode::kInvalidIndex)
    return nullptr;
  return scene_graph->nodes + node->parent_index;
}

// Add Transform -----------------------------------------------------------------------------------

namespace {

constexpr uint64_t kNotFound = (uint64_t)-1;

// Will mark the slot as used.
template <size_t SIZE>
inline uint64_t FindOpenSlot(std::bitset<SIZE>& bitset, uint64_t size) {
  for (uint64_t i = 0; i < size; i++) {
    if (!bitset[i]) {
      bitset[i] = 1;
      return i;
    }
  }

  return kNotFound;
}

}  // namespace

SceneNode* AddNode(SceneGraph* scene_graph, uint32_t parent_index) {
  ASSERT(scene_graph->count < kSceneGraphSize);

  uint64_t index = FindOpenSlot(scene_graph->used, kSceneGraphSize);
  ASSERT(index < kSceneGraphSize);

  // Clear the node.
  SceneNode* node = scene_graph->nodes + index;
  *node = {};
  node->index = index;

  node->parent_index = parent_index;
  if (parent_index != SceneNode::kInvalidIndex) {
    ASSERT(scene_graph->used[parent_index]);
    SceneNode* parent = scene_graph->nodes + parent_index;
    parent->children.push_back(index);
  }

  scene_graph->count++;
  return node;
}

// Delete Transform --------------------------------------------------------------------------------

namespace {

}  // namespace


void DeleteTransform(SceneGraph* scene_graph, uint32_t index, uint32_t parent_index) {
  ASSERT(scene_graph->count > 0);
  ASSERT(scene_graph->used[index]);

  SceneNode* node = scene_graph->nodes + index;

  // Mark the transform as not used anymore.
  scene_graph->used[index] = 0;
  scene_graph->count--;

  // All children should be deleted too.
  for (uint32_t child_index : node->children) {
    // We don't want to delete the parent again.
    DeleteTransform(scene_graph, child_index, SceneNode::kInvalidIndex);
  }

  // Check if we need to update the parent as well.
  if (parent_index != SceneNode::kInvalidIndex) {
    ASSERT(scene_graph->used[parent_index]);
    SceneNode* parent = scene_graph->nodes + parent_index;

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

// Update ------------------------------------------------------------------------------------------

namespace {

void UpdateNode(SceneGraph* scene_graph, SceneNode* node, SceneNode* parent) {
  Update(node, parent);

  for (uint32_t child_index : node->children) {
    SceneNode* child = scene_graph->nodes + child_index;
    UpdateNode(scene_graph, child, node);
  }
}

}  // namespace

void Update(SceneGraph* scene_graph) {
  if (scene_graph->count == 0)
    return;

  SceneNode* current_node = scene_graph->nodes;
  UpdateNode(scene_graph, current_node, nullptr);
}

}  // namespace rothko
