// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/commands.h"

#include <sstream>

#include "rothko/graphics/graphics.h"

namespace rothko {

const char* ToString(RenderCommandType type) {
  switch (type) {
    case RenderCommandType::kNop: return "Nop";
    case RenderCommandType::kClearFrame: return "Clear Frame";
    case RenderCommandType::kPushConfig: return "Push Config";
    case RenderCommandType::kPopConfig: return "Pop Config";
    case RenderCommandType::kPushCamera: return "Push Camera";
    case RenderCommandType::kPopCamera: return "Pop Camera";
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

// Clear Frame -------------------------------------------------------------------------------------

std::string ToString(const ClearFrame& clear_frame) {
  std::stringstream ss;
  ss << std::boolalpha;

  bool clear_color = GetClearColor(clear_frame.flags);
  ss << "Clear Color :" << clear_color;
  if (clear_color)
    ss << " (color: " << std::hex << clear_frame.color << ")";

  ss << ", Clear depth: " << GetClearDepth(clear_frame.flags);

  return ss.str();
}

ClearFrame ClearFrame::FromColor(Color c) {
  ClearFrame clear = {};
  clear.color = c.r << 24 | c.g << 16 | c.b << 8 | c.a;
  return clear;
}

// Config Renderer ---------------------------------------------------------------------------------

std::string ToString(const PushConfig& push_config) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Viewport base: " << ToString(push_config.viewport_pos)
     << ", size: " << ToString(push_config.viewport_size);
  return ss.str();
}

std::string ToString(const PopConfig&) {
  return "Pop config";
}

// Camera ------------------------------------------------------------------------------------------

std::string ToString(const PushCamera& push_camera) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Projection: " << ToString(push_camera.projection) << std::endl
     << "View: " << ToString(push_camera.view);
  return ss.str();
}

std::string ToString(const PopCamera&) {
  return "Pop camera";
}

// Render Mesh -------------------------------------------------------------------------------------

std::string ToString(const RenderMesh& render_mesh) {
  std::stringstream ss;
  ss << std::boolalpha;
  ss << "Mesh: " << render_mesh.mesh->name << ", Shader: " << render_mesh.shader->config.name
     << std::endl;

  ss << "Indices= Offset: " << render_mesh.indices_offset
     << ", Count: " << render_mesh.indices_count << std::endl;

  ss << std::hex;
  for (uint32_t i = 0; i < std::size(render_mesh.ubo_data); i++) {
    uint8_t* ubo_data = render_mesh.ubo_data[i];
    if (ubo_data)
      ss << "UBO " << i << ": 0x" << std::hex << (void*)ubo_data << std::endl;
  }
  ss << std::dec;

  for (size_t i = 0; i < render_mesh.textures.size(); i++) {
    ss << "Tex" << i << ": " << render_mesh.textures[i]->name << ", ";
  }
  if (!render_mesh.textures.empty())
    ss << std::endl;

  if (GetScissorTest(render_mesh.flags)) {
    ss << "Scissor= Pos: " << ToString(render_mesh.scissor_pos)
       << ", Size: " << ToString(render_mesh.scissor_size) << std::endl;
  }

  ss << "Blend: " << GetBlendEnabled(render_mesh.flags) << ", "
     << "Cull Faces: " << GetCullFaces(render_mesh.flags) << ", "
     << "Depth mask: " << GetDepthMask(render_mesh.flags) << ", "
     << "Depth test: " << GetDepthTest(render_mesh.flags) << ", "
     << "Wireframe: " << GetWireframeMode(render_mesh.flags);

  return ss.str();
};

// Render Command ----------------------------------------------------------------------------------

std::string ToString(const RenderCommand& command) {
  std::stringstream ss;
  ss << "Type: " << ToString(command.type()) << std::endl;
  switch (command.type()) {
    case RenderCommandType::kNop:
      ss << "Nop";
    case RenderCommandType::kClearFrame:
      ss << ToString(command.GetClearFrame());
      break;
    case RenderCommandType::kPushConfig:
      ss << ToString(command.GetPushConfig());
      break;
    case RenderCommandType::kPopConfig:
      ss << ToString(command.GetPopConfig());
      break;
    case RenderCommandType::kRenderMesh:
      ss << ToString(command.GetRenderMesh());
      break;
    case RenderCommandType::kPushCamera:
      ss << ToString(command.GetPushCamera());
    case RenderCommandType::kPopCamera:
      ss << ToString(command.GetPopCamera());
    case RenderCommandType::kLast:
      break;
  }

  return ss.str();
}



}  // namespace rothko
