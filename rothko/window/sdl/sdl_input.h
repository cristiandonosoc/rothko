// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/window/sdl/sdl_definitions.h"

namespace rothko {

struct Input;

namespace sdl {

void HandleKeysDown(Input*);
void HandleKeyUpEvent(const SDL_KeyboardEvent& key_event, Input* input);
void HandleMouse(Input*);
void HandleMouseWheelEvent(const SDL_MouseWheelEvent&, Input*);

}  // namespace sdl
}  // namespace rothko
