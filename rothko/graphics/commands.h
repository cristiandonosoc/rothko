// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <variant>

#include "rothko/containers/vector.h"
#include "rothko/math/math.h"
#include "rothko/utils/macros.h"
#include "rothko/graphics/mesh.h"

namespace rothko {

// Render Commands
// =================================================================================================
//
// Render commands represent an "action" the renderer must do. Some of the actions are "clear the
// screen", "render this mesh", "set viewport", etc. Note that some of this actions are stateful,
// meaning that they *will* affect how the renderer will behave in the future (think setting the
// viewport).

struct Camera;
struct Mesh;
struct Shader;
struct Texture;

// Render Actions ----------------------------------------------------------------------------------

enum class RenderCommandType {
  kClearFrame,
  kConfigRenderer,
  kRenderMesh,
  kPushCamera,
  kLast,
};
const char* ToString(RenderCommandType);
uint32_t ToSize(RenderCommandType);

enum class PrimitiveType {
  kLines,
  kLineStrip,
  kTriangles,
  kLast,
};
const char* ToString(PrimitiveType);

struct ClearFrame {
  static constexpr RenderCommandType kType = RenderCommandType::kClearFrame;

  bool clear_depth = true;
  bool clear_color = true;
  uint32_t color = 0;   // One byte per color, RGBA (R = 24, G = 16, B = 8, A = 0).
};
std::string ToString(const ClearFrame&);

struct ConfigRenderer {
  static constexpr RenderCommandType kType = RenderCommandType::kConfigRenderer;

  Int2 viewport_base = {};
  Int2 viewport_size = {};  // If non-zero, makes the renderer consider this the viewport size;
};
std::string ToString(const ConfigRenderer&);

// Sets the current camera state on the renderer. This persists to other calls.
struct PushCamera {
  static constexpr RenderCommandType kType = RenderCommandType::kPushCamera;

  Vec3 camera_pos;
  Mat4 projection;
  Mat4 view;
};
std::string ToString(const PushCamera&);

// RenderMesh --------------------------------------------------------------------------------------

namespace lines {

inline float GetLineWidth(uint64_t ctx) { return (float)(ctx & 0b111u); }
inline uint64_t SetLineWidth(uint64_t ctx, int width) { return ctx & (width | ~(uint64_t)0b111); }

}  // namespace

namespace line_strip {

// Value to use for reseting the indices lookup (basically glPrimitiveRestartIndex).
constexpr Mesh::IndexType kPrimitiveReset = (Mesh::IndexType)-2;

inline uint32_t GetRestartIndex(uint64_t ctx) { return (uint32_t)(ctx & (uint32_t)-1); }
inline uint64_t SetRestartIndex(uint64_t ctx, uint32_t i) {
  uint64_t mask = (i | ~(uint64_t)(uint32_t)-1);
  return ctx & mask;
}

}  // namespace line_strip

// Represents all the information needed to render a mesh. It provides the mesh, texture, uniform
// and whatnot. The renderer can be clever about re-using state (like if two consecutive render
// mesh commands use the same shader), but it's not obligated to do that.
struct RenderMesh {
  static constexpr RenderCommandType kType = RenderCommandType::kRenderMesh;

  Mesh* mesh = nullptr;
  Shader* shader = nullptr;

  PrimitiveType primitive_type = PrimitiveType::kLast;

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

#define GENERATE_COMMAND(Command, getter)                                  \
  RenderCommand(Command cmd) { SetRenderCommand(std::move(cmd)); }         \
  RenderCommand& operator=(Command cmd) {                                  \
    SetRenderCommand(std::move(cmd));                                      \
    return *this;                                                          \
  }                                                                        \
  Command& Get##Command() { return std::get<Command>(data_); }             \
  const Command& Get##Command() const { return std::get<Command>(data_); } \
  bool getter() const { return type_ == RenderCommandType::k##Command; }

struct RenderCommand {
  RenderCommand() = default;
  DEFAULT_COPY_AND_ASSIGN(RenderCommand);
  DEFAULT_MOVE_AND_ASSIGN(RenderCommand);

  RenderCommandType type() const { return type_; }

  GENERATE_COMMAND(ClearFrame, is_clear_frame);
  GENERATE_COMMAND(ConfigRenderer, is_config_renderer);
  GENERATE_COMMAND(PushCamera, is_push_camera);
  GENERATE_COMMAND(RenderMesh, is_render_mesh);

 private:
  RenderCommandType type_ = RenderCommandType::kLast;
  std::variant<ClearFrame, ConfigRenderer, PushCamera, RenderMesh> data_;

  template <typename T>
  void SetRenderCommand(T t) {
    type_ = T::kType;
    data_ = std::move(t);
  }
};

std::string ToString(const RenderCommand&);

}  // namespace rothko
