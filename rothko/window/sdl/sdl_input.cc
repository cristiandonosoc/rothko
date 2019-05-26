// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/window/sdl/sdl_input.h"

#include "rothko/input/input.h"

namespace rothko {
namespace sdl {

#define SET_SDL_KEY(array, state, sdl_key, key) \
  array[GET_KEY(key)] = !!state[SDL_SCANCODE_##sdl_key];

void HandleKeysDown(Input* input) {
  const uint8_t* key_state = SDL_GetKeyboardState(0);
  SET_SDL_KEY(input->down_this_frame, key_state, UP, Up);
  SET_SDL_KEY(input->down_this_frame, key_state, DOWN, Down);
  SET_SDL_KEY(input->down_this_frame, key_state, LEFT, Left);
  SET_SDL_KEY(input->down_this_frame, key_state, RIGHT, Right);
  SET_SDL_KEY(input->down_this_frame, key_state, A, A);
  SET_SDL_KEY(input->down_this_frame, key_state, B, B);
  SET_SDL_KEY(input->down_this_frame, key_state, C, C);
  SET_SDL_KEY(input->down_this_frame, key_state, D, D);
  SET_SDL_KEY(input->down_this_frame, key_state, E, E);
  SET_SDL_KEY(input->down_this_frame, key_state, F, F);
  SET_SDL_KEY(input->down_this_frame, key_state, G, G);
  SET_SDL_KEY(input->down_this_frame, key_state, H, H);
  SET_SDL_KEY(input->down_this_frame, key_state, I, I);
  SET_SDL_KEY(input->down_this_frame, key_state, J, J);
  SET_SDL_KEY(input->down_this_frame, key_state, K, K);
  SET_SDL_KEY(input->down_this_frame, key_state, L, L);
  SET_SDL_KEY(input->down_this_frame, key_state, M, M);
  SET_SDL_KEY(input->down_this_frame, key_state, N, N);
  SET_SDL_KEY(input->down_this_frame, key_state, O, O);
  SET_SDL_KEY(input->down_this_frame, key_state, P, P);
  SET_SDL_KEY(input->down_this_frame, key_state, Q, Q);
  SET_SDL_KEY(input->down_this_frame, key_state, R, R);
  SET_SDL_KEY(input->down_this_frame, key_state, S, S);
  SET_SDL_KEY(input->down_this_frame, key_state, T, T);
  SET_SDL_KEY(input->down_this_frame, key_state, U, U);
  SET_SDL_KEY(input->down_this_frame, key_state, V, V);
  SET_SDL_KEY(input->down_this_frame, key_state, W, W);
  SET_SDL_KEY(input->down_this_frame, key_state, X, X);
  SET_SDL_KEY(input->down_this_frame, key_state, Y, Y);
  SET_SDL_KEY(input->down_this_frame, key_state, Z, Z);
  SET_SDL_KEY(input->down_this_frame, key_state, 0, 0);
  SET_SDL_KEY(input->down_this_frame, key_state, 1, 1);
  SET_SDL_KEY(input->down_this_frame, key_state, 2, 2);
  SET_SDL_KEY(input->down_this_frame, key_state, 3, 3);
  SET_SDL_KEY(input->down_this_frame, key_state, 4, 4);
  SET_SDL_KEY(input->down_this_frame, key_state, 5, 5);
  SET_SDL_KEY(input->down_this_frame, key_state, 6, 6);
  SET_SDL_KEY(input->down_this_frame, key_state, 7, 7);
  SET_SDL_KEY(input->down_this_frame, key_state, 8, 8);
  SET_SDL_KEY(input->down_this_frame, key_state, 9, 9);
  SET_SDL_KEY(input->down_this_frame, key_state, GRAVE, Backquote);
  SET_SDL_KEY(input->down_this_frame, key_state, PAGEUP, PageUp);
  SET_SDL_KEY(input->down_this_frame, key_state, PAGEDOWN, PageDown);
  SET_SDL_KEY(input->down_this_frame, key_state, HOME, Home);
  SET_SDL_KEY(input->down_this_frame, key_state, END, End);
  SET_SDL_KEY(input->down_this_frame, key_state, INSERT, Insert);
  SET_SDL_KEY(input->down_this_frame, key_state, DELETE, Delete);
  SET_SDL_KEY(input->down_this_frame, key_state, BACKSPACE, Backspace);
  SET_SDL_KEY(input->down_this_frame, key_state, SPACE, Space);
  SET_SDL_KEY(input->down_this_frame, key_state, RETURN, Enter);
  SET_SDL_KEY(input->down_this_frame, key_state, ESCAPE, Escape);
  SET_SDL_KEY(input->down_this_frame, key_state, TAB, Tab);

  auto mod_state = SDL_GetModState();
  input->down_this_frame[GET_KEY(Ctrl)] = mod_state & KMOD_CTRL;
  input->down_this_frame[GET_KEY(Alt)] = mod_state & KMOD_ALT;
  input->down_this_frame[GET_KEY(Shift)] = mod_state & KMOD_SHIFT;
  input->down_this_frame[GET_KEY(Super)] = mod_state & KMOD_GUI;

  // Set the control state
  // TODO(Cristian): This should not be here.
  input->up = KeyDown(input, Key::kUp);
  input->down = KeyDown(input, Key::kDown);
  input->left = KeyDown(input, Key::kLeft);
  input->right = KeyDown(input, Key::kRight);
}

void HandleKeyUpEvent(const SDL_KeyboardEvent&, Input*) {}

void HandleMouse(Input* input) {
  auto mouse_state = SDL_GetMouseState(&input->mouse.pos.x,
                                       &input->mouse.pos.y);
  input->mouse.left = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
  input->mouse.middle = mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  input->mouse.right = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);

  input->mouse_offset = input->mouse.pos - input->prev_mouse.pos;
}

void HandleMouseWheelEvent(const SDL_MouseWheelEvent& wheel_event,
                           Input* input) {
  input->mouse.wheel.x = wheel_event.x;
  input->mouse.wheel.y = wheel_event.y;
}

}  // namespace sdl
}  // namespace rothko
