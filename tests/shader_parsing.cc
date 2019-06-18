// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/common/shader.h>

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {

// CalculateUBOLayout ----------------------------------------------------------

namespace {

Uniform CreateUniform(UniformType type) {
  Uniform uniform;
  uniform.type = type;
  return uniform;
}

}  // namespace

TEST_CASE("CalculateUniformLayout") {
  UniformBufferObject ubo;
  ubo.uniforms = {
    CreateUniform(UniformType::kFloat),
    CreateUniform(UniformType::kVec3),
    CreateUniform(UniformType::kMat4),
    CreateUniform(UniformType::kVec2),
    CreateUniform(UniformType::kVec3),
    CreateUniform(UniformType::kVec4),
    CreateUniform(UniformType::kBool),
    CreateUniform(UniformType::kBool),
    CreateUniform(UniformType::kMat4),
    CreateUniform(UniformType::kInt),
  };

  REQUIRE(CalculateUBOLayout(&ubo));

  // Float.
  REQUIRE(ubo.uniforms[0].alignment == 4);
  REQUIRE(ubo.uniforms[0].offset == 0);
  REQUIRE(ubo.uniforms[0].size == 4);

  // Vec3.
  REQUIRE(ubo.uniforms[1].alignment == 16);
  REQUIRE(ubo.uniforms[1].offset == 16);
  REQUIRE(ubo.uniforms[1].size == 12);

  // Mat4.
  REQUIRE(ubo.uniforms[2].alignment == 16);
  REQUIRE(ubo.uniforms[2].offset == 32);
  REQUIRE(ubo.uniforms[2].size == 64);

  // Vec2.
  REQUIRE(ubo.uniforms[3].alignment == 8);
  REQUIRE(ubo.uniforms[3].offset == 96);
  REQUIRE(ubo.uniforms[3].size == 8);

  // Vec3.
  REQUIRE(ubo.uniforms[4].alignment == 16);
  REQUIRE(ubo.uniforms[4].offset == 112);
  REQUIRE(ubo.uniforms[4].size == 12);

  // Vec4.
  REQUIRE(ubo.uniforms[5].alignment == 16);
  REQUIRE(ubo.uniforms[5].offset == 128);
  REQUIRE(ubo.uniforms[5].size == 16);

  // Bool.
  REQUIRE(ubo.uniforms[6].alignment == 4);
  REQUIRE(ubo.uniforms[6].offset == 144);
  REQUIRE(ubo.uniforms[6].size == 4);

  // Bool.
  REQUIRE(ubo.uniforms[7].alignment == 4);
  REQUIRE(ubo.uniforms[7].offset == 148);
  REQUIRE(ubo.uniforms[7].size == 4);

  // Mat4.
  REQUIRE(ubo.uniforms[8].alignment == 16);
  REQUIRE(ubo.uniforms[8].offset == 160);
  REQUIRE(ubo.uniforms[8].size == 64);

  // Int.
  REQUIRE(ubo.uniforms[9].alignment == 4);
  REQUIRE(ubo.uniforms[9].offset == 224);
  REQUIRE(ubo.uniforms[9].size == 4);
}

// ParseSubShader --------------------------------------------------------------

static constexpr char kValidShader[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_uv;

out vec3 color;
out vec2 uv;

// Uniforms --------------------------------------------------------------------

//#Camera:proj:mat4
//#Camera:view:mat4
layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

//#VertUniforms:model:mat4
UNIFORM_BLOCK(VertUniforms) {
  mat4 model;
} uniforms;

//#AllUniforms:Uniform0:float
//#AllUniforms:Uniform1:vec3
//#AllUniforms:Uniform2:mat4
//#AllUniforms:Uniform3:vec2
//#AllUniforms:Uniform4:vec3
//#AllUniforms:Uniform5:vec4
//#AllUniforms:Uniform6:bool
//#AllUniforms:Uniform7:bool
//#AllUniforms:Uniform8:mat4
//#AllUniforms:Uniform9:int

// Code ------------------------------------------------------------------------

void main() {
  gl_Position = camera.proj * camera.view * uniforms.model * vec4(in_pos, 1.0);
  color = in_color;
  uv = in_uv;
}
)";

TEST_CASE("ParseShader") {
  SubShaderParseResult result;
  REQUIRE(ParseSubShader(kValidShader, &result));

  REQUIRE(result.ubos.size() == 3u);

  {
    REQUIRE(result.ubos[0].name == "AllUniforms");
    auto& uniforms = result.ubos[0].uniforms;
    REQUIRE(uniforms.size() == 10u);

    CHECK(uniforms[0].name == "Uniform0");
    CHECK(uniforms[0].type == UniformType::kFloat);
    CHECK(uniforms[0].alignment == 4);
    CHECK(uniforms[0].offset == 0);
    CHECK(uniforms[0].size == 4);

    CHECK(uniforms[1].name == "Uniform1");
    CHECK(uniforms[1].type == UniformType::kVec3);
    CHECK(uniforms[1].alignment == 16);
    CHECK(uniforms[1].offset == 16);
    CHECK(uniforms[1].size == 12);

    CHECK(uniforms[2].name == "Uniform2");
    CHECK(uniforms[2].type == UniformType::kMat4);
    CHECK(uniforms[2].alignment == 16);
    CHECK(uniforms[2].offset == 32);
    CHECK(uniforms[2].size == 64);

    CHECK(uniforms[3].name == "Uniform3");
    CHECK(uniforms[3].type == UniformType::kVec2);
    CHECK(uniforms[3].alignment == 8);
    CHECK(uniforms[3].offset == 96);
    CHECK(uniforms[3].size == 8);

    CHECK(uniforms[4].name == "Uniform4");
    CHECK(uniforms[4].type == UniformType::kVec3);
    CHECK(uniforms[4].alignment == 16);
    CHECK(uniforms[4].offset == 112);
    CHECK(uniforms[4].size == 12);

    CHECK(uniforms[5].name == "Uniform5");
    CHECK(uniforms[5].type == UniformType::kVec4);
    CHECK(uniforms[5].alignment == 16);
    CHECK(uniforms[5].offset == 128);
    CHECK(uniforms[5].size == 16);

    CHECK(uniforms[6].name == "Uniform6");
    CHECK(uniforms[6].type == UniformType::kBool);
    CHECK(uniforms[6].alignment == 4);
    CHECK(uniforms[6].offset == 144);
    CHECK(uniforms[6].size == 4);

    CHECK(uniforms[7].name == "Uniform7");
    CHECK(uniforms[7].type == UniformType::kBool);
    CHECK(uniforms[7].alignment == 4);
    CHECK(uniforms[7].offset == 148);
    CHECK(uniforms[7].size == 4);

    CHECK(uniforms[8].name == "Uniform8");
    CHECK(uniforms[8].type == UniformType::kMat4);
    CHECK(uniforms[8].alignment == 16);
    CHECK(uniforms[8].offset == 160);
    CHECK(uniforms[8].size == 64);

    CHECK(uniforms[9].name == "Uniform9");
    CHECK(uniforms[9].type == UniformType::kInt);
    CHECK(uniforms[9].alignment == 4);
    CHECK(uniforms[9].offset == 224);
    CHECK(uniforms[9].size == 4);
  }

  {
    REQUIRE(result.ubos[1].name == "Camera");
    auto& uniforms = result.ubos[1].uniforms;
    REQUIRE(uniforms.size() == 2u);

    CHECK(uniforms[0].name == "proj");
    CHECK(uniforms[0].type == UniformType::kMat4);
    CHECK(uniforms[0].alignment == 16);
    CHECK(uniforms[0].size == 64);
    CHECK(uniforms[1].name == "view");
    CHECK(uniforms[1].type == UniformType::kMat4);
    CHECK(uniforms[1].alignment == 16);
    CHECK(uniforms[1].size == 64);
  }

  {
    REQUIRE(result.ubos[2].name == "VertUniforms");
    auto& uniforms = result.ubos[2].uniforms;

    CHECK(uniforms[0].name == "model");
    CHECK(uniforms[0].type == UniformType::kMat4);
    CHECK(uniforms[0].alignment == 16);
    CHECK(uniforms[0].size == 64);
  }

}

}  // namespace test
}  // namespace rothko
