
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
    case WidgetOperation::kTranslate: return ImGuizmo::OPERATION::TRANSLATE;
    case WidgetOperation::kRotate: return ImGuizmo::OPERATION::ROTATE;
    case WidgetOperation::kScale: return ImGuizmo::OPERATION::SCALE;
  }

  NOT_REACHED();
  return ImGuizmo::OPERATION::TRANSLATE;
}

bool IsZero(const Transform& t) {
  return IsZero(t.position) && IsZero(t.rotation) && IsZero(t.scale);
}

}  // namespace

Transform TransformWidget(WidgetOperation op,
                          const PushCamera& camera,
                          const Transform& source,
                          const Transform* parent) {
  // Scale is only local otherwise it resets the rotation.
  ImGuizmo::MODE imguizmo_mode = op == WidgetOperation::kScale ? ImGuizmo::MODE::LOCAL :
                                                                 ImGuizmo::MODE::WORLD;
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

}  // namespace rothko
