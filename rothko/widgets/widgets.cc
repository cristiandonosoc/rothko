// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/widgets/widgets.h"

#include <stddef.h>
#include <third_party/imguizmo/ImGuizmo.h>

#include "rothko/graphics/commands.h"
#include "rothko/math/math.h"
#include "rothko/scene/transform.h"

namespace rothko {

namespace {

ImGuizmo::OPERATION GetImGuizmoOperation(WidgetOperation op) {
  switch (op) {
    case WidgetOperation::kTranslation: return ImGuizmo::OPERATION::TRANSLATE;
    case WidgetOperation::kRotation: return ImGuizmo::OPERATION::ROTATE;
    case WidgetOperation::kScale: return ImGuizmo::OPERATION::SCALE;
  }

  NOT_REACHED();
  return ImGuizmo::OPERATION::TRANSLATE;
}

bool IsZero(const Transform& t) {
  return IsZero(t.position) && IsZero(t.rotation) && IsZero(t.scale);
}

}  // namespace

void TransformWidget(WidgetOperation op, const PushCamera& camera, Transform* transform) {
  ImGuizmo::Manipulate((float*)&camera.view,
                       (float*)&camera.projection,
                       GetImGuizmoOperation(op),
                       ImGuizmo::MODE::WORLD,
                       (float*)&transform->world_matrix);

  *transform = TransformMatrixToTransform(transform->world_matrix);
}

Transform TransformWidget(WidgetOperation op, const PushCamera& camera, const Transform& source) {
  Mat4 m = source.world_matrix;
  ImGuizmo::Manipulate((float*)&camera.view,
                       (float*)&camera.projection,
                       GetImGuizmoOperation(op),
                       ImGuizmo::MODE::WORLD,
                       (float*)&m);

  Transform dest = TransformMatrixToTransform(m);
  Transform diff = source - dest;
  if (IsZero(diff))
    return source;
  return source - diff;
}

}  // namespace rothko
