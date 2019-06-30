// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <variant>

#include "rothko/containers/vector.h"
#include "rothko/math/vec.h"

namespace rothko {

struct Camera;
struct Mesh;
struct Shader;
struct Texture;

union RenderAction;

// Render Actions --------------------------------------------------------------

struct ClearRenderAction {
  bool clear_depth = true;

  bool clear_color = true;
  Vec3 color;
};

struct MeshRenderAction {
  Mesh* mesh = nullptr;

  Int2 scissor_pos = {};
  Int2 scissor_size = {};

  uint32_t indices_offset = 0;
  uint32_t indices_size = 0;

  PerFrameVector<uint8_t*> vert_ubos;
  PerFrameVector<uint8_t*> frag_ubos;
  PerFrameVector<Texture*> textures;
};

// Render Command --------------------------------------------------------------

enum class RenderCommandType {
  kClear,
  kMesh,
  kLast,
};

struct RenderCommand {
  RenderCommandType type = RenderCommandType::kLast;

  // Config.
  bool blend_enabled = false;
  bool cull_faces = true;
  bool depth_test = true;
  bool scissor_test = false;
  bool wireframe_mode = false;

  Shader* shader = nullptr;

  std::variant<ClearRenderAction, PerFrameVector<MeshRenderAction>> data;

  // Getters.
  bool is_clear_action() const;
  ClearRenderAction& ClearAction();
  const ClearRenderAction& ClearAction() const;

  bool is_mesh_actions() const;
  PerFrameVector<MeshRenderAction>& MeshActions();
  const PerFrameVector<MeshRenderAction>& MeshActions() const;
};

}  // namespace rothko
