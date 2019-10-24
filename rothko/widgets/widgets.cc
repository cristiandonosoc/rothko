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

}  // namespace

void TransformWidget(WidgetOperation op, const PushCamera& push_camera,Transform* transform) {
  ImGuizmo::Manipulate((float*)&push_camera.view,
                       (float*)&push_camera.projection,
                       GetImGuizmoOperation(op),
                       ImGuizmo::MODE::WORLD,
                       (float*)&transform->world_matrix);

  TransformMatrixToTransform(transform->world_matrix, transform);
}

}  // namespace rothko
