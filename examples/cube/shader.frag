// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

in vec4 color;

layout (location = 0) out vec4 out_color;

// Code ------------------------------------------------------------------------

void main() {
  out_color = color;
}
