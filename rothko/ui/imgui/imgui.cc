// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/ui/imgui/imgui.h"

#include "rothko/input/input.h"
#include "rothko/platform/timing.h"
#include "rothko/ui/imgui/def.h"
#include "rothko/ui/imgui/imgui_renderer.h"
#include "rothko/window/window.h"

// Imgui config verification check.
// This is to verify that our imgui config wasn't overwritten by a imgui update.
// Our imgui config is in rothko/ui/imgui/warhol_imgui_config.h
// That config should be placed in third_party/imgui/imconfig.h
/* #ifndef ROTHKO_IMGUI_CONFIG */
/* #error No warhol imgui config loaded. Is third_party/imgui/imconfig.h correct? */
/* #endif */

namespace rothko {
namespace imgui {

// Init --------------------------------------------------------------------------------------------

namespace {

static MouseCursor gMouseCursors[ImGuiMouseCursor_COUNT] = {};

void MapIO(ImGuiIO* io) {
  // Keyboard mapping. ImGui will use those indices to peek into the
  // io.KeysDown[] array.
  io->KeyMap[ImGuiKey_Tab] = GET_KEY(Tab);
  io->KeyMap[ImGuiKey_LeftArrow] = GET_KEY(Left);
  io->KeyMap[ImGuiKey_RightArrow] = GET_KEY(Right);
  io->KeyMap[ImGuiKey_UpArrow] = GET_KEY(Up);
  io->KeyMap[ImGuiKey_DownArrow] = GET_KEY(Down);
  io->KeyMap[ImGuiKey_PageUp] = GET_KEY(PageUp);
  io->KeyMap[ImGuiKey_PageDown] = GET_KEY(PageDown);
  io->KeyMap[ImGuiKey_Home] = GET_KEY(Home);
  io->KeyMap[ImGuiKey_End] = GET_KEY(End);
  io->KeyMap[ImGuiKey_Insert] = GET_KEY(Insert);
  io->KeyMap[ImGuiKey_Delete] = GET_KEY(Delete);
  io->KeyMap[ImGuiKey_Backspace] = GET_KEY(Backspace);
  io->KeyMap[ImGuiKey_Space] = GET_KEY(Space);
  io->KeyMap[ImGuiKey_Enter] = GET_KEY(Enter);
  io->KeyMap[ImGuiKey_Escape] = GET_KEY(Escape);
  io->KeyMap[ImGuiKey_A] = GET_KEY(A);
  io->KeyMap[ImGuiKey_C] = GET_KEY(C);
  io->KeyMap[ImGuiKey_V] = GET_KEY(V);
  io->KeyMap[ImGuiKey_X] = GET_KEY(X);
  io->KeyMap[ImGuiKey_Y] = GET_KEY(Y);
  io->KeyMap[ImGuiKey_Z] = GET_KEY(Z);

  gMouseCursors[ImGuiMouseCursor_Arrow] = MouseCursor::kArrow;
  gMouseCursors[ImGuiMouseCursor_TextInput] = MouseCursor::kIbeam;
  gMouseCursors[ImGuiMouseCursor_ResizeAll] = MouseCursor::kSizeAll;
  gMouseCursors[ImGuiMouseCursor_ResizeNS] = MouseCursor::kSizeNS;
  gMouseCursors[ImGuiMouseCursor_ResizeEW] = MouseCursor::kSizeWE;
  gMouseCursors[ImGuiMouseCursor_ResizeNESW] = MouseCursor::kSizeNESW;
  gMouseCursors[ImGuiMouseCursor_ResizeNWSE] = MouseCursor::kSizeNWSE;
  gMouseCursors[ImGuiMouseCursor_Hand] = MouseCursor::kHand;
}

}  // namespace

bool InitImgui(Renderer* renderer, ImguiContext* imgui) {
  if (Valid(imgui)) {
    LOG(ERROR, "Imgui context already initialized.");
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  imgui->io = &ImGui::GetIO();
  ASSERT(imgui->io);
  MapIO(imgui->io);

  imgui->imgui_renderer.io = imgui->io;
  if (!InitImguiRenderer(&imgui->imgui_renderer, renderer, imgui->io))
    return false;
  return true;
}

// Shutdown ----------------------------------------------------------------------------------------

ImguiContext::~ImguiContext() {
  if (!Valid(this))
    return;
  ImGui::DestroyContext();
}

// Start Frame -------------------------------------------------------------------------------------

namespace {

void RestartKeys(Window* window, Input* input, ImGuiIO* io) {
  for (size_t i = 0; i < ARRAY_SIZE(io->KeysDown); i++) {
    io->KeysDown[i] = false;
  }

  for (size_t i = 0; i < kMaxKeys; i++) {
    if (input->down_this_frame[i])
      io->KeysDown[i] = true;
  }

  io->KeyCtrl = input->down_this_frame[GET_KEY(Ctrl)];
  io->KeyAlt = input->down_this_frame[GET_KEY(Alt)];
  io->KeyShift = input->down_this_frame[GET_KEY(Shift)];
  io->KeySuper = input->down_this_frame[GET_KEY(Super)];

  // Pass in the text input characters.
  io->AddInputCharactersUTF8(window->utf8_chars_inputted);

  // Update Mouse.
  io->MousePos = { (float)input->mouse.pos.x, (float)input->mouse.pos.y };
  io->MouseDown[0] = input->mouse.left;
  io->MouseDown[1] = input->mouse.right;
  io->MouseDown[2] = input->mouse.middle;
  io->MouseWheelH += input->mouse.wheel.x;
  io->MouseWheel += input->mouse.wheel.y;

  // TODO(Cristian): Update cursors.

  ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
  if (io->MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
    window->backend->ShowCursor(false);
  } else {
    window->backend->SetMouseCursor(gMouseCursors[imgui_cursor]);
    window->backend->ShowCursor(true);
  }
}

}  // namespace

void StartFrame(ImguiContext* imgui, Window* window, Time* time, Input* input) {
  ASSERT(Valid(window));
  ASSERT(Valid(imgui));

  imgui->io->DisplaySize = {(float)window->width, (float)window->height};
  imgui->io->DisplayFramebufferScale = {1.0f, 1.0f};

  // TODO(Cristian): Obtain time delta from platform!
  imgui->io->DeltaTime = time->frame_delta;

  RestartKeys(window, input, imgui->io);

  ImGui::NewFrame();
}

// End Frame -------------------------------------------------------------------


PerFrameVector<RenderCommand> EndFrame(ImguiContext* imgui) {
  ASSERT(Valid(imgui));
  // Will finalize the draw data needed for getting the draw lists for getting
  // the render command.
  ImGui::Render();
  return ImguiGetRenderCommands(&imgui->imgui_renderer);
}

}  // namespace imgui
}  // namespace rothko

