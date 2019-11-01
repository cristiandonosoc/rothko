// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/scene/transform.h"

namespace rothko {

struct PushCamera;
struct Transform;

enum class WidgetOperation {
  kTranslate,
  kRotate,
  kScale,
};

enum class TransformKind {
  kLocal,
  kGlobal,
};

// Returns the diff transform. If |parent| is set, it will be used to reverse the parent's
// transformation in order to give out the correct delta (otherwise the parent's transformation will
// be added to the delta and the widget will be wrong. Normally moves the object to infinity).
Transform TransformWidget(WidgetOperation, TransformKind,
                          const PushCamera&,
                          const Transform& source,
                          const Transform* parent = nullptr);

// Aliases.

inline Transform TranslateWidget(TransformKind transform_kind,
                                 const PushCamera& push_camera,
                                 const Transform& source,
                                 const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kTranslate, transform_kind, push_camera, source, parent);
}

inline Transform RotateWidget(TransformKind transform_kind, const PushCamera& push_camera,
                              const Transform& source,
                              const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kRotate, transform_kind, push_camera, source, parent);
}
inline Transform ScaleWidget(const PushCamera& push_camera,
                             const Transform& source,
                             const Transform* parent = nullptr) {
  // Scale is only local otherwise it resets the rotation.
  return TransformWidget(WidgetOperation::kScale, TransformKind::kLocal,
                         push_camera, source, parent);
}

}  // namespace rothko
