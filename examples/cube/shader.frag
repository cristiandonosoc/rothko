// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes --------------------------------------------------------------------------------------

in vec2 f_uv;
in vec4 f_color;

layout (location = 0) out vec4 out_color;

// Uniforms ----------------------------------------------------------------------------------------

uniform sampler2D tex0;
uniform sampler2D tex1;

// Code --------------------------------------------------------------------------------------------

void main() {
  out_color = mix(texture(tex0, f_uv), texture(tex1, f_uv), 0.5f) * f_color;
}
