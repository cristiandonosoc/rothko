// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/commands.h"

#include <sstream>

#include "rothko/graphics/graphics.h"

namespace rothko {

const char* ToString(RenderCommandType type) {
  switch (type) {
    case RenderCommandType::kClearFrame: return "Clear Frame";
    case RenderCommandType::kConfigRenderer: return "Config Renderer";
    case RenderCommandType::kPushCamera: return "Push Camera";
    case RenderCommandType::kRenderMesh: return "Render Mesh";
    case RenderCommandType::kLast: return "<last>";
  }

  NOT_REACHED();
  return "<unknown>";
}

const char* ToString(PrimitiveType type) {
  switch (type) {
    case PrimitiveType::kLines: return "Lines";
    case PrimitiveType::kLineStrip: return "Line Strip";
    case PrimitiveType::kTriangles: return "Triangles";
    case PrimitiveType::kLast: return "<last>";
  }

  NOT_REACHED();
  return "<unknown>";
}

// Clear Action-------------------------------------------------------------------------------------

std::string ToString(const ClearFrame& clear_frame) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Clear Color :" << clear_frame.clear_color;
  if (clear_frame.clear_color)
    ss << " (color: " << std::hex << clear_frame.color << ")";
  ss << ", Clear depth: " << clear_frame.clear_depth;

  return ss.str();
}

ClearFrame ClearFrame::FromColor(uint32_t color) {
  ClearFrame clear = {};
  clear.color = color;
  return clear;
}

// Config Renderer ---------------------------------------------------------------------------------

std::string ToString(const ConfigRenderer& config_renderer) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Viewport base: " << ToString(config_renderer.viewport_base)
     << ", size: " << ToString(config_renderer.viewport_size);
  return ss.str();
}

// Push Camera -------------------------------------------------------------------------------------

std::string ToString(const PushCamera& push_camera) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Projection: " << ToString(push_camera.projection) << std::endl
     << "View: " << ToString(push_camera.view);
  return ss.str();
}

// Render Mesh -------------------------------------------------------------------------------------

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
  ss << "Type: " << ToString(command.type()) << std::endl;
  switch (command.type()) {
    case RenderCommandType::kClearFrame:
      ss << ToString(command.GetClearFrame());
      break;
    case RenderCommandType::kConfigRenderer:
      ss << ToString(command.GetConfigRenderer());
      break;
    case RenderCommandType::kRenderMesh:
      ss << ToString(command.GetRenderMesh());
      break;
    case RenderCommandType::kPushCamera:
      ss << ToString(command.GetPushCamera());
    case RenderCommandType::kLast:
      break;
  }

  return ss.str();
}



}  // namespace rothko
