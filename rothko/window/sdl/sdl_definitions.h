// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Use this header to include SDL definitions into your code. In general, no
// code should need this as only the SDL window should be aware of SDL. But
// even within the SDL window, they should only include SDL definitions from
// here, in order to be able to tweak any special includes only once within this
// place, instead of having to hunt the definitions over.

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
