// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/scene/transform.h"

#include "rothko/utils/macros.h"

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

// In general use the convenience versions at the end.

// Returns the diff transform. If |parent| is set, it will be used to reverse the parent's
// transformation in order to give out the correct delta (otherwise the parent's transformation will
// be added to the delta and the widget will be wrong. Normally moves the object to infinity).
NO_DISCARD Transform TransformWidget(WidgetOperation,
                                     TransformKind,
                                     const PushCamera&,
                                     const Transform& source,
                                     const Transform* parent = nullptr);

// Aliases.

NO_DISCARD inline Transform TranslateWidget(TransformKind transform_kind,
                                            const PushCamera& push_camera,
                                            const Transform& source,
                                            const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kTranslate, transform_kind, push_camera, source, parent);
}

NO_DISCARD inline Transform RotateWidget(TransformKind transform_kind,
                                         const PushCamera& push_camera,
                                         const Transform& source,
                                         const Transform* parent = nullptr) {
  return TransformWidget(WidgetOperation::kRotate, transform_kind, push_camera, source, parent);
}

NO_DISCARD inline Transform ScaleWidget(const PushCamera& push_camera,
                                        const Transform& source,
                                        const Transform* parent = nullptr) {
  // Scale is only local otherwise it resets the rotation.
  return TransformWidget(WidgetOperation::kScale, TransformKind::kLocal,
                         push_camera, source, parent);
}

// Convenience overloads ---------------------------------------------------------------------------

inline void TranslateWidget(TransformKind transform_kind,
                            const PushCamera& push_camera,
                            Transform* source,
                            const Transform* parent = nullptr) {
  *source = TranslateWidget(transform_kind, push_camera, *source, parent);
}

inline void RotateWidget(TransformKind transform_kind,
                         const PushCamera& push_camera,
                         Transform* source,
                         const Transform* parent = nullptr) {
  *source = RotateWidget(transform_kind, push_camera, *source, parent);
}

// Scale is only local otherwise it resets the rotation.
inline void ScaleWidget(const PushCamera& push_camera,
                        Transform* source,
                        const Transform* parent = nullptr) {
  *source = ScaleWidget(push_camera, *source, parent);
}

}  // namespace rothko
