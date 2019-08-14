// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/math/math.h>

// This is meant to be a holder API for getting specialized shaders for each kind of  supported
// renderer within Rothko.
//
// All functions are present here for linking but if a particular renderer is not supported
// (say OpenGL is not present), calling that function will assert.

namespace rothko {

struct Shader;

namespace imgui {

struct ImguiUBO {
  Mat4 projection;
  Mat4 view;
};

Shader GetOpenGLImguiShader();

}  // namespace imgui
}  // namespace rothko
