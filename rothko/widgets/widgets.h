// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace rothko {

struct PushCamera;
struct Transform;

enum class WidgetOperation {
  kTranslation,
  kRotation,
  kScale,
};

// NOTE: Requires ImGuizmo (and hence ImGui to be loaded).
//       Uses |transform.world_matrix|, so it probably should be updated already.
void TransformWidget(WidgetOperation, const PushCamera&, Transform*);

}  // namespace rothko
