// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/transform.h"

#include "rothko/logging/logging.h"

namespace rothko {

// Transform ---------------------------------------------------------------------------------------

Mat4 GetLocalTransformMatrix(const Transform& transform) {
  Mat4 result = {};
  result = Translate(transform.position);
  result *= Rotate({1, 0, 0}, transform.rotation.x);
  result *= Rotate({0, 1, 0}, transform.rotation.y);
  result *= Rotate({0, 0, 1}, transform.rotation.z);
  result *= Scale(transform.scale);

  return result;
}

Mat4 GetWorlTransformMatrix(const SceneGraph& scene_graph, const Transform& transform) {
  ASSERT(transform.index != Transform::kInvalidIndex);
  Mat4 parent_transform = transform.parent == Transform::kInvalidIndex
                              ? Mat4::Identity()
                              : scene_graph.transforms[transform.index].world_matrix_;
  Mat4 local_transform = GetLocalTransformMatrix(transform);

  return local_transform * parent_transform;
}

void Update(const SceneGraph& scene_graph, Transform* transform) {
  ASSERT(transform->index != Transform::kInvalidIndex);
  transform->world_matrix_ = GetWorlTransformMatrix(scene_graph, *transform);
}

// SceneGraph --------------------------------------------------------------------------------------

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

Transform* AddTransform(SceneGraph* scene_graph) {
  ASSERT(scene_graph->count < kSceneGraphSize);

  uint64_t index = FindOpenSlot(scene_graph->used, kSceneGraphSize);
  ASSERT(index < kSceneGraphSize);

  // Clear the transform.
  Transform* transform = scene_graph->transforms + index;
  *transform = {};
  transform->index = index;

  return transform;
}

}  // namespace rothko
