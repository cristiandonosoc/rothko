// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"
#include "rothko/ui/imgui/imgui_shaders.h"
#include "rothko/utils/macros.h"

struct ImGuiIO;

namespace rothko {

struct RenderCommand;
struct Renderer;
struct RenderMesh;

namespace imgui {

struct ImguiContext;

struct ImguiRenderer {
  Mesh mesh;
  Shader shader;
  Texture font_texture;

  ImGuiIO* io = nullptr;
  Renderer* renderer = nullptr;  // Must outlive.
};

inline bool Valid(ImguiRenderer* r) { return !!r->renderer && !!r->io; }

// Requires ImGuiRenderer.io to be already set.
bool InitImguiRenderer(ImguiRenderer*, Renderer*, ImGuiIO*);

PerFrameVector<RenderCommand> ImguiGetRenderCommands(ImguiRenderer*);

}  // namespace imgui
}  // namespace rothko
