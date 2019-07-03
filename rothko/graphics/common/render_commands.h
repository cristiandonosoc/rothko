// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <variant>

#include "rothko/containers/vector.h"
#include "rothko/math/math.h"

namespace rothko {

struct Camera;
struct Mesh;
struct Shader;
struct Texture;

union RenderAction;

// Render Actions ----------------------------------------------------------------------------------

struct ClearFrame {
  bool clear_depth = true;
  bool clear_color = true;
  uint32_t color;   // One byte per color.
};

struct RenderMesh {
  Mesh* mesh = nullptr;
  Shader* shader = nullptr;

  // Config.
  // TODO(Cristian): This could me move to a bit-field.
  bool blend_enabled = false;
  bool cull_faces = true;
  bool depth_test = true;
  bool scissor_test = false;
  bool wireframe_mode = false;

  Int2 scissor_pos = {};
  Int2 scissor_size = {};

  uint32_t indices_offset = 0;
  uint32_t indices_size = 0;

  // The size of the UBO is given by the description of the corresponding shader.
  // It is the responsability of the caller that these buffers match.
  uint8_t* vert_ubo_data = nullptr;
  uint8_t* frag_ubo_data = nullptr;

  PerFrameVector<Texture*> textures;
};

// Render Command ----------------------------------------------------------------------------------

enum class RenderCommandType {
  kClear,
  kMesh,
  kLast,
};

struct RenderCommand {
  RenderCommandType type = RenderCommandType::kLast;

  std::variant<ClearFrame, RenderMesh> data;

  // Getters.
  bool is_clear_frame() const;
  ClearFrame& GetClearFrame();
  const ClearFrame& GetClearFrame() const;

  bool is_render_mesh() const;
  RenderMesh& GetRenderMesh();
  const RenderMesh& GetRenderMesh() const;
};

}  // namespace rothko
