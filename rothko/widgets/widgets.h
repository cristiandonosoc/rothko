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

// Returns the diff transform. If |parent| is set, it will be used to reverse the parent's
// transformation in order to give out the correct delta (otherwise the parent's transformation will
// be added to the delta and the widget will be wrong. Normally moves the object to infinity).
Transform TransformWidget(WidgetOperation,
                          const PushCamera&,
                          const Transform& source,
                          const Transform* parent = nullptr);

// Aliases.

inline Transform TranslateWidget(const PushCamera& push_camera,
                                 const Transform& source,
                                 const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kTranslate, push_camera, source, parent);
}

inline Transform RotateWidget(const PushCamera& push_camera,
                              const Transform& source,
                              const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kRotate, push_camera, source, parent);
}
inline Transform ScaleWidget(const PushCamera& push_camera,
                             const Transform& source,
                             const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kScale, push_camera, source, parent);
}

}  // namespace rothko
