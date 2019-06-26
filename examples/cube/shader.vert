// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 color;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

// Code ------------------------------------------------------------------------

void main() {
  gl_Position = camera.proj * camera.view * vec4(in_pos, 1.0);
  color = in_color;
}
