// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/transform.h"

#include "rothko/logging/logging.h"
#include "rothko/scene/scene_graph.h"

namespace rothko {

// Transform ---------------------------------------------------------------------------------------

Mat4 GetLocalTransformMatrix(const Transform& transform) {
  Mat4 result = Mat4::Identity();
  result *= Translate(transform.position);
  result *= Rotate({1, 0, 0}, transform.rotation.x);
  result *= Rotate({0, 1, 0}, transform.rotation.y);
  result *= Rotate({0, 0, 1}, transform.rotation.z);
  result *= Scale(transform.scale);

  return result;
}

Mat4 GetWorlTransformMatrix(const SceneGraph* scene_graph, const Transform& transform) {
  if (!scene_graph || transform.parent_index == Transform::kInvalidIndex)
    return GetLocalTransformMatrix(transform);

  Mat4 parent_transform = transform.parent_index == Transform::kInvalidIndex
                              ? Mat4::Identity()
                              : scene_graph->transforms[transform.index].world_matrix;
  Mat4 local_transform = GetLocalTransformMatrix(transform);

  return local_transform * parent_transform;
}

void Update(const SceneGraph* scene_graph, Transform* transform) {
  transform->world_matrix = GetWorlTransformMatrix(scene_graph, *transform);
}

}  // namespace rothko
