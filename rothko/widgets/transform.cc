
// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/transform.h"

#include <stddef.h>
#include <third_party/imguizmo/ImGuizmo.h>

#include "rothko/graphics/commands.h"
#include "rothko/math/math.h"
#include "rothko/ui/imgui.h"
#include "rothko/widgets/widgets.h"

namespace rothko {

namespace {

ImGuizmo::OPERATION GetImGuizmoOperation(WidgetOperation op) {
  switch (op) {
    case WidgetOperation::kTranslate: return ImGuizmo::OPERATION::TRANSLATE;
    case WidgetOperation::kRotate: return ImGuizmo::OPERATION::ROTATE;
    case WidgetOperation::kScale: return ImGuizmo::OPERATION::SCALE;
  }

  NOT_REACHED();
  return ImGuizmo::OPERATION::TRANSLATE;
}

ImGuizmo::MODE GetImGuizmoMode(TransformKind transform_kind) {
  switch (transform_kind) {
    case TransformKind::kGlobal: return ImGuizmo::MODE::WORLD;
    case TransformKind::kLocal: return ImGuizmo::MODE::LOCAL;
  }

  NOT_REACHED();
  return ImGuizmo::MODE::WORLD;
}

bool IsZero(const Transform& t) {
  return IsZero(t.position) && IsZero(t.rotation) && IsZero(t.scale);
}

}  // namespace

Transform TransformWidget(WidgetOperation op,
                          TransformKind transform_kind,
                          const PushCamera& camera,
                          const Transform& source,
                          const Transform* parent) {
  // Scale is only local otherwise it resets the rotation.
  ImGuizmo::MODE imguizmo_mode = GetImGuizmoMode(transform_kind);
  if (op == WidgetOperation::kScale)
    imguizmo_mode = ImGuizmo::MODE::LOCAL;

  Mat4 temp = source.world_matrix;
  ImGuizmo::Manipulate((float*)&camera.view,
                       (float*)&camera.projection,
                       GetImGuizmoOperation(op),
                       imguizmo_mode,
                       (float*)&temp);

  // We need to remove the parent transformation.
  Mat4 m = temp;
  if (parent) {
    Mat4 inverse = Inverse(parent->world_matrix);
    m = inverse * temp;
  };

  Transform dest = TransformMatrixToTransform(m);

  Transform diff = source - dest;
  if (IsZero(diff))
    return source;
  return source - diff;
}

// Imgui -------------------------------------------------------------------------------------------

void TransformImguiWidget(const Transform& transform) {
  ImGui::InputFloat3("Position", (float*)&transform.position, "%.3f", ImGuiInputTextFlags_ReadOnly);
  ImGui::InputFloat3("Rotation", (float*)&transform.rotation, "%.3f", ImGuiInputTextFlags_ReadOnly);
  ImGui::InputFloat3("Scale", (float*)&transform.scale, "%.3f", ImGuiInputTextFlags_ReadOnly);
}

}  // namespace rothko
