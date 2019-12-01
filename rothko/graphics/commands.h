// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <variant>

#include "rothko/containers/vector.h"
#include "rothko/graphics/color.h"
#include "rothko/graphics/definitions.h"
#include "rothko/graphics/mesh.h"
#include "rothko/math/math.h"
#include "rothko/utils/macros.h"

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
  kNop,           // Don't do anything.
  kClearFrame,
  kPushConfig,
  kPopConfig,
  kRenderMesh,
  kPushCamera,
  kPopCamera,
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

// No op -------------------------------------------------------------------------------------------

struct Nop {
  static constexpr RenderCommandType kType = RenderCommandType::kNop;
};

// Clear Frame -------------------------------------------------------------------------------------

BIT_FLAG(ClearColor, 0);
BIT_FLAG(ClearDepth, 1);

struct ClearFrame {
  static constexpr RenderCommandType kType = RenderCommandType::kClearFrame;

  static ClearFrame FromColor(Color color);

  uint32_t flags = kClearColor | kClearDepth;
  uint32_t color = 0;   // One byte per color, RGBA (R = 24, G = 16, B = 8, A = 0).
};
std::string ToString(const ClearFrame&);

// Config ------------------------------------------------------------------------------------------

constexpr int kMaxConfigCount = 4;

struct PushConfig{
  static constexpr RenderCommandType kType = RenderCommandType::kPushConfig;

  Int2 viewport_pos = {};
  Int2 viewport_size = {};  // If non-zero, makes the renderer consider this the viewport size;
};
std::string ToString(const PushConfig&);

struct PopConfig {
  static constexpr RenderCommandType kType = RenderCommandType::kPopConfig;
};
std::string ToString(const PopConfig&);

// Camera ------------------------------------------------------------------------------------------

// Sets the current camera state on the renderer. This persists to other calls.
// |kMaxCameraCount| establishes how many cameras can be pushed at the same time.
constexpr int kMaxCameraCount = 4;

struct PushCamera {
  static constexpr RenderCommandType kType = RenderCommandType::kPushCamera;

  Vec3 camera_pos;
  Mat4 projection;
  Mat4 view;
};
std::string ToString(const PushCamera&);

// Popping when there is no camera is an error.
struct PopCamera {
  static constexpr RenderCommandType kType = RenderCommandType::kPopCamera;
};
std::string ToString(const PopCamera&);

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

BIT_FLAG(BlendEnabled, 0);
BIT_FLAG(CullFaces, 1);
BIT_FLAG(DepthMask, 2);
BIT_FLAG(DepthTest, 3);
BIT_FLAG(ScissorTest, 4);
BIT_FLAG(WireframeMode, 5);

// Represents all the information needed to render a mesh. It provides the mesh, texture, uniform
// and whatnot. The renderer can be clever about re-using state (like if two consecutive render
// mesh commands use the same shader), but it's not obligated to do that.
struct RenderMesh {
  static constexpr RenderCommandType kType = RenderCommandType::kRenderMesh;

  const Mesh* mesh = nullptr;
  const Shader* shader = nullptr;

  PrimitiveType primitive_type = PrimitiveType::kLast;

  // Config.
  uint32_t flags = kCullFaces | kDepthMask | kDepthMask | kDepthTest;

  Int2 scissor_pos = {};
  Int2 scissor_size = {};

  uint32_t indices_offset = 0;
  uint32_t indices_count = 0;

  // The size of the UBO is given by the description of the corresponding shader.
  // It is the responsability of the caller that these buffers match.
  uint8_t* ubo_data[kMaxUBOs] = {};

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

  GENERATE_COMMAND(Nop, is_nop);
  GENERATE_COMMAND(ClearFrame, is_clear_frame);
  GENERATE_COMMAND(PushConfig, is_push_config);
  GENERATE_COMMAND(PopConfig, is_pop_config);
  GENERATE_COMMAND(PushCamera, is_push_camera);
  GENERATE_COMMAND(PopCamera, is_pop_camera);
  GENERATE_COMMAND(RenderMesh, is_render_mesh);

 private:
  RenderCommandType type_ = RenderCommandType::kLast;
  std::variant<Nop, ClearFrame, PushConfig, PopConfig, PushCamera, PopCamera, RenderMesh> data_;

  template <typename T>
  void SetRenderCommand(T t) {
    type_ = T::kType;
    data_ = std::move(t);
  }
};

std::string ToString(const RenderCommand&);

}  // namespace rothko
