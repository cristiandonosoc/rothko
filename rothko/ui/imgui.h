// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Proxy header to be able to use imgui easily.
#include "rothko/math/math.h"

#define IM_VEC2_CLASS_EXTRA         \
  ImVec2(const ::rothko::Vec2& f) { \
    x = f.x;                        \
    y = f.y;                        \
  }                                 \
  operator ::rothko::Vec2() const { return ::rothko::Vec2{x, y}; }

#define IM_VEC4_CLASS_EXTRA         \
  ImVec4(const ::rothko::Vec4& f) { \
    x = f.x;                        \
    y = f.y;                        \
    z = f.z;                        \
    w = f.w;                        \
  }                                 \
  operator ::rothko::Vec4() const { return ::rothko::Vec4{x, y, z, w}; }

#include "rothko/ui/imgui/def.h"
#include "rothko/ui/imgui/imgui.h"
#include "rothko/ui/imgui/imgui_windows.h"

namespace rothko {
namespace imgui {

inline ImVec2 ToImVec2(Int2 v) { return {(float)v.x, (float)v.y}; }
inline ImVec2 ToImVec2(Vec2 v) { return {v.x, v.y}; }

inline ImVec4 ToImVec4(Int4 v) { return {(float)v.x, (float)v.y, (float)v.z, (float)v.w}; }
inline ImVec4 ToImVec4(Vec4 v) { return {v.x, v.y, v.z, v.w}; }

inline Vec2 ToVec2(ImVec2 v) { return {(float)v.x, (float)v.y}; }
inline Vec4 ToVec4(ImVec4 v) { return {v.x, v.y, v.z, v.w}; }

inline void ReadOnlyTextInput(const char* label, const char* str, size_t size) {
  ImGui::InputText(label, (char*)str, size, ImGuiInputTextFlags_ReadOnly);
}

inline void ReadOnlyTextInput(const char* label, const std::string& str) {
  ImGui::InputText(label, (char*)str.c_str(), str.length() + 1);
}


}  // namespace imgui
}  // namespace rothko
