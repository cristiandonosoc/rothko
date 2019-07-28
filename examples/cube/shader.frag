// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes --------------------------------------------------------------------------------------

in vec2 in_uv;
in vec4 in_color;

layout (location = 0) out vec4 out_color;

// Uniforms ----------------------------------------------------------------------------------------

uniform sampler2D tex0;

// Code --------------------------------------------------------------------------------------------

void main() {
  out_color = texture(tex0, in_uv) * in_color;
}
