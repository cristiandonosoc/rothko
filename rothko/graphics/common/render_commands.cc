// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/render_commands.h"

namespace rothko {

namespace {

template <typename T>
void SetRenderCommand(RenderCommand* command, T t) {
  command->type_ = T::kType;
  command->data_ = t;
}

}  // namespace

// Clear Action------------------------------------------------------------------------------------

RenderCommand::RenderCommand(ClearFrame clear_frame) {
  SetRenderCommand(this, std::move(clear_frame));
}

RenderCommand& RenderCommand::operator=(ClearFrame clear_frame) {
  SetRenderCommand(this, std::move(clear_frame));
  return *this;
}

ClearFrame& RenderCommand::GetClearFrame() {
  if (!is_clear_frame())
    SetRenderCommand(this, ClearFrame());
  return std::get<ClearFrame>(data_);
}

const ClearFrame& RenderCommand::GetClearFrame() const { return std::get<ClearFrame>(data_); }

// MeshActions ------------------------------------------------------------------------------------

RenderCommand::RenderCommand(RenderMesh render_mesh) {
  SetRenderCommand(this, std::move(render_mesh));
}

RenderCommand& RenderCommand::operator=(RenderMesh render_mesh) {
  SetRenderCommand(this, std::move(render_mesh));
  return *this;
}

RenderMesh& RenderCommand::GetRenderMesh() {
  if (!is_render_mesh())
    SetRenderCommand(this, RenderMesh());
  return std::get<RenderMesh>(data_);
}

const RenderMesh& RenderCommand::GetRenderMesh() const { return std::get<RenderMesh>(data_); }

}  // namespace rothko
