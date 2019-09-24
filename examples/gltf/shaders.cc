// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shaders.h"

namespace rothko {
namespace gltf {

namespace {

constexpr char kVertex[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec3 f_normal;
out vec2 f_uv;

void main(){
  mat3 mvp = camera_proj * camera_view;
	gl_Position = mvp * vec4(in_pos, 1);
	f_normal = normalize(mat3(mvp) * in_normal);
	f_uv = in_uv;
}

)";

constexpr char kFragment[] = R"(
#version 330 core
in vec3 f_normal;
in vec2 f_uv;

uniform sampler2D tex;
uniform vec3 sun_position;
uniform vec3 sun_color;

out vec4 color;
void main() {
	float lum = max(dot(f_normal, normalize(sun_position)), 0.0);
	color = texture(tex, f_uv) * vec4((0.3 + 0.7 * lum) * sun_color, 1.0);
}
)";

}  // namespace


std::unique_ptr<Shader> CreateNormalShader(Renderer*);

}  // namespace gltf
}  // namespace rothko

