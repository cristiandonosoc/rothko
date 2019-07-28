// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes --------------------------------------------------------------------------------------

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 f_uv;
out vec4 f_color;

// Uniforms ----------------------------------------------------------------------------------------

layout (std140) uniform Uniforms {
  mat4 proj;
  mat4 view;
  mat4 model;
};

// Code --------------------------------------------------------------------------------------------

void main() {
  gl_Position = proj * view * model * vec4(in_pos, 1.0);
  f_uv = in_uv;
  f_color = in_color;
}
