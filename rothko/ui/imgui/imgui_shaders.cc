// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/ui/imgui/imgui_shaders.h"

#include <memory>

#include "rothko/graphics/renderer.h"
#include "rothko/graphics/shader.h"
#include "rothko/logging/logging.h"

namespace rothko {
namespace imgui {

// =================================================================================================
// OpenGL
// =================================================================================================

#ifdef ROTHKO_OPENGL_ENABLED

const char kOpenGLVertex[] = R"(
layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec4 color;
out vec2 uv;

void main() {
  gl_Position = camera_proj * camera_view * vec4(in_pos.xy, 0, 1.0f);
  color = in_color;
  uv = in_uv;
}
)";

const char kOpenGLFragment[] = R"(
in vec4 color;
in vec2 uv;

uniform sampler2D tex_sampler;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = color * texture(tex_sampler, uv);
}
)";

std::unique_ptr<Shader> GetOpenGLImguiShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "Imgui Shader";
  config.vertex_type = VertexType::k2dUVColor;
  config.texture_count = 1;

  auto vert_src = CreateVertexSource(kOpenGLVertex);
  auto frag_src = CreateFragmentSource(kOpenGLFragment);

  return RendererStageShader(renderer, config, vert_src, frag_src);
}

#else

Shader GetOpenGLImguiShader() {
  NOT_REACHED_MSG("OpenGL support not enabled.");
  return {};
}

#endif

}  // namespace imgui
}  // namespace rothko
