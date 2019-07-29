// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <variant>

#include "rothko/containers/vector.h"
#include "rothko/math/math.h"
#include "rothko/utils/macros.h"

namespace rothko {

struct Camera;
struct Mesh;
struct Shader;
struct Texture;

union RenderAction;

// Render Actions ----------------------------------------------------------------------------------

enum class RenderCommandType {
  kClear,
  kMesh,
  kConfigRenderer,
  kLast,
};
const char* ToString(RenderCommandType);
uint32_t ToSize(RenderCommandType);

struct ClearFrame {
  static constexpr RenderCommandType kType = RenderCommandType::kClear;

  bool clear_depth = true;
  bool clear_color = true;
  uint32_t color = 0;   // One byte per color, RGBA (R = 24, G = 16, B = 8, A = 0).
};
std::string ToString(const ClearFrame&);

struct ConfigRenderer {
  static constexpr RenderCommandType kType = RenderCommandType::kConfigRenderer;

  Int2 viewport = {};  // If non-zero, makes the renderer consider this the viewport size;
};
std::string ToString(const ConfigRenderer&);

struct RenderMesh {
  static constexpr RenderCommandType kType = RenderCommandType::kMesh;

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
std::string ToString(const RenderMesh&);

// Render Command ----------------------------------------------------------------------------------

struct RenderCommand {
  RenderCommand() = default;
  DEFAULT_COPY_AND_ASSIGN(RenderCommand);
  DEFAULT_MOVE_AND_ASSIGN(RenderCommand);

  RenderCommand(ClearFrame);
  RenderCommand& operator=(ClearFrame);

  RenderCommand(ConfigRenderer);
  RenderCommand& operator=(ConfigRenderer);

  RenderCommand(RenderMesh);
  RenderCommand& operator=(RenderMesh);

  RenderCommandType type() const { return type_; }
  bool is_clear_frame() const { return type_ == RenderCommandType::kClear; }
  bool is_config_renderer() const { return type_ == RenderCommandType::kConfigRenderer; }
  bool is_render_mesh() const { return type_ == RenderCommandType::kMesh; }

  // Getters.
  ClearFrame& GetClearFrame();
  const ClearFrame& GetClearFrame() const;

  ConfigRenderer& GetConfigRenderer();
  const ConfigRenderer& GetConfigRenderer() const;

  RenderMesh& GetRenderMesh();
  const RenderMesh& GetRenderMesh() const;

  // "pseudo"-private.
  RenderCommandType type_ = RenderCommandType::kLast;
  std::variant<ClearFrame, ConfigRenderer, RenderMesh> data_;
};
std::string ToString(const RenderCommand&);

}  // namespace rothko
