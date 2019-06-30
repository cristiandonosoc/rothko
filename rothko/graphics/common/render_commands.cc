// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/render_commands.h"

namespace rothko {

// Clear Action------------------------------------------------------------------------------------

bool RenderCommand::is_clear_action() const {
  return std::holds_alternative<ClearRenderAction>(data);
}

ClearRenderAction& RenderCommand::ClearAction() {
  if (!is_clear_action())
    data = ClearRenderAction();

  return std::get<ClearRenderAction>(data);
}

const ClearRenderAction& RenderCommand::ClearAction() const {
  return std::get<ClearRenderAction>(data);
}

// MeshActions ------------------------------------------------------------------------------------

bool RenderCommand::is_mesh_actions() const {
  return std::holds_alternative<PerFrameVector<MeshRenderAction>>(data);
}

PerFrameVector<MeshRenderAction>& RenderCommand::MeshActions() {
  if (!is_mesh_actions())
    data = PerFrameVector<MeshRenderAction>();

  return std::get<PerFrameVector<MeshRenderAction>>(data);
}

const PerFrameVector<MeshRenderAction>& RenderCommand::MeshActions() const {
  return std::get<PerFrameVector<MeshRenderAction>>(data);
}

}  // namespace rothko
