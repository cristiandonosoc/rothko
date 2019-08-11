// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Proxy header to be able to use imgui easily.
#include "rothko/math/math.h"
#include "rothko/ui/imgui/def.h"
#include "rothko/ui/imgui/imgui.h"
#include "rothko/ui/imgui/imgui_windows.h"

namespace rothko {

inline ImVec2 ToImVec(Vec2 v) { return {v.x, v.y}; }

}  // namespace rothko
