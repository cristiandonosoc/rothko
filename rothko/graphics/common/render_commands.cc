// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/render_commands.h"

namespace rothko {

// Clear Action------------------------------------------------------------------------------------

bool RenderCommand::is_clear_frame() const { return std::holds_alternative<ClearFrame>(data); }

ClearFrame& RenderCommand::GetClearFrame() {
  if (!is_clear_frame())
    data = ClearFrame();

  return std::get<ClearFrame>(data);
}

const ClearFrame& RenderCommand::GetClearFrame() const { return std::get<ClearFrame>(data); }

// MeshActions ------------------------------------------------------------------------------------

bool RenderCommand::is_render_mesh() const { return std::holds_alternative<RenderMesh>(data); }

RenderMesh& RenderCommand::GetRenderMesh() {
  if (!is_render_mesh())
    data = RenderMesh();

  return std::get<RenderMesh>(data);
}

const RenderMesh& RenderCommand::GetRenderMesh() const { return std::get<RenderMesh>(data); }

}  // namespace rothko
