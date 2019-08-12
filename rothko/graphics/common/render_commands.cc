// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/render_commands.h"

#include <sstream>

#include "rothko/graphics/graphics.h"

namespace rothko {

namespace {

template <typename T>
void SetRenderCommand(RenderCommand* command, T t) {
  command->type_ = T::kType;
  command->data_ = t;
}

}  // namespace

const char* ToString(RenderCommandType type) {
  switch (type) {
    case RenderCommandType::kClear: return "Clear";
    case RenderCommandType::kConfigRenderer: return "Config Renderer";
    case RenderCommandType::kMesh: return "Mesh";
    case RenderCommandType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

// Clear Action-------------------------------------------------------------------------------------

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

std::string ToString(const ClearFrame& clear_frame) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Clear Color :" << clear_frame.clear_color;
  if (clear_frame.clear_color)
    ss << " (color: " << std::hex << clear_frame.color << ")";
  ss << ", Clear depth: " << clear_frame.clear_depth;

  return ss.str();
}

// Config Renderer ---------------------------------------------------------------------------------

RenderCommand::RenderCommand(ConfigRenderer config_renderer) {
  SetRenderCommand(this, std::move(config_renderer));
}

RenderCommand& RenderCommand::operator=(ConfigRenderer config_renderer) {
  SetRenderCommand(this, std::move(config_renderer));
  return *this;
}

ConfigRenderer& RenderCommand::GetConfigRenderer() {
  if (!is_config_renderer())
    SetRenderCommand(this, ConfigRenderer());
  return std::get<ConfigRenderer>(data_);
}

const ConfigRenderer& RenderCommand::GetConfigRenderer() const {
  return std::get<ConfigRenderer>(data_);
}

std::string ToString(const ConfigRenderer& config_renderer) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Viewport base: " << ToString(config_renderer.viewport_base)
     << ", size: " << ToString(config_renderer.viewport_size);
  return ss.str();
}

// Render Mesh -------------------------------------------------------------------------------------

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

namespace {

}  // namespace

const RenderMesh& RenderCommand::GetRenderMesh() const { return std::get<RenderMesh>(data_); }

std::string ToString(const RenderMesh& render_mesh) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Mesh: " << render_mesh.mesh->name << ", Shader: " << render_mesh.shader->name << std::endl;

  ss << "Indices= Offset: " << render_mesh.indices_offset << ", Size: " << render_mesh.indices_size
     << std::endl;

  ss << std::hex;
  if (render_mesh.vert_ubo_data)
    ss << "Vert UBO: 0x" << (void*)render_mesh.vert_ubo_data << ", ";
  if (render_mesh.frag_ubo_data)
    ss << "Frag UBO: 0x" << (void*)render_mesh.frag_ubo_data;
  ss << std::endl;
  ss << std::dec;

  for (size_t i = 0; i < render_mesh.textures.size(); i++) {
    ss << "Tex" << i << ": " << render_mesh.textures[i]->name << ", ";
  }
  if (!render_mesh.textures.empty())
    ss << std::endl;

  if (render_mesh.scissor_test) {
    ss << "Scissor= Pos: " << ToString(render_mesh.scissor_pos)
       << ", Size: " << ToString(render_mesh.scissor_size) << std::endl;
  }

  ss << "Blend: " << render_mesh.blend_enabled << ", "
     << "Cull Faces: " << render_mesh.cull_faces << ", "
     << "Depth test: " << render_mesh.depth_test << ", "
     << "Wireframe: " << render_mesh.wireframe_mode;

  return ss.str();
};

// Render Command ----------------------------------------------------------------------------------

std::string ToString(const RenderCommand& command) {
  std::stringstream ss;
  ss << "Type: " << ToString(command.type_) << std::endl;
  switch (command.type_) {
    case RenderCommandType::kClear:
      ss << ToString(command.GetClearFrame());
      break;
    case RenderCommandType::kConfigRenderer:
      ss << ToString(command.GetConfigRenderer());
      break;
    case RenderCommandType::kMesh:
      ss << ToString(command.GetRenderMesh());
      break;
    case RenderCommandType::kLast:
      break;
  }

  return ss.str();
}

}  // namespace rothko
