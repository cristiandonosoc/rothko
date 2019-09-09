// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/ui/imgui.h"
#include "rothko/ui/imgui/imgui_renderer.h"
#include "rothko/utils/macros.h"

struct ImGuiIO;

namespace rothko {

struct Input;
struct RenderCommand;
struct Renderer;
struct Time;
struct Window;

namespace imgui {

struct ImguiContext {
  RAII_CONSTRUCTORS(ImguiContext);

  // This struct represents a handle to the imgui system. Not owning, must outlive.
  ImGuiIO* io = nullptr;

  bool keyboard_captured = false;
  bool mouse_captured = false;

  Vec2 font_size = {};

  ImguiRenderer imgui_renderer;
};

inline bool Valid(ImguiContext* imgui) { return !!imgui->io && Valid(&imgui->imgui_renderer); }

// Both the renderer and window must outlive the imgui context.
bool InitImgui(Renderer* renderer, ImguiContext*);

void StartFrame(ImguiContext*, Window*, Time*, Input*);

// Gets the command to be passed down to the renderer.
// IMPORTANT: StartFrame *has* to be called each frame before this.
PerFrameVector<RenderCommand> EndFrame(ImguiContext*);

}  // namespace imgui
}  // namespace rothko
