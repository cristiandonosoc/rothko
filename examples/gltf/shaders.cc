// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shaders.h"

#include <rothko/graphics/graphics.h>

namespace rothko {
namespace gltf {

namespace {

constexpr char kVertex[] = R"(
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out vec3 f_normal;
out vec2 f_uv;

void main(){
  mat4 mvp = camera_proj * camera_view;
	gl_Position = mvp * vec4(in_pos, 1);
	f_normal = normalize(mat3(mvp) * in_normal);
	f_uv = in_uv;
}

)";

constexpr char kFragment[] = R"(
in vec3 f_normal;
in vec2 f_uv;

layout (location = 0) out vec4 out_color;

uniform sampler2D tex0;

void main() {
	/* float lum = max(dot(f_normal, normalize(sun_position)), 0.0); */
	/* color = texture(tex, f_uv) * vec4((0.3 + 0.7 * lum) * sun_color, 1.0); */
  out_color = texture(tex0, f_uv);
}
)";

}  // namespace


Shader CreateNormalShader(Renderer* renderer) {
  Shader shader = {};
  shader.name = "gltf-shader";
  shader.vertex_type = VertexType::k3dNormalUV;
  shader.texture_count = 1;

  shader.vert_src = CreateVertexSource(kVertex);
  shader.frag_src = CreateFragmentSource(kFragment);

  if (!RendererStageShader(renderer, &shader))
    return {};
  return shader;
}

}  // namespace gltf
}  // namespace rothko

