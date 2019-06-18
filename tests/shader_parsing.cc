// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/common/shader.h>

#include <third_party/catch2/catch.hpp>

namespace rothko {
namespace test {

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

//#AllUniforms:Bool:bool
//#AllUniforms:Int:int
//#AllUniforms:Float:float
//#AllUniforms:Int2:int2
//#AllUniforms:Int3:int3
//#AllUniforms:Int4:int4
//#AllUniforms:Uint2:uint2
//#AllUniforms:Uint3:uint3
//#AllUniforms:Uint4:uint4
//#AllUniforms:Vec2:vec2
//#AllUniforms:Vec3:vec3
//#AllUniforms:Vec4:vec4
//#AllUniforms:Ivec2:ivec2
//#AllUniforms:Ivec3:ivec3
//#AllUniforms:Ivec4:ivec4
//#AllUniforms:Uvec2:uvec2
//#AllUniforms:Uvec3:uvec3
//#AllUniforms:Uvec4:uvec4
//#AllUniforms:Mat4:mat4

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
    REQUIRE(uniforms.size() == 19u);

    CHECK(uniforms[0].name == "Bool");
    CHECK(uniforms[0].type == UniformType::kBool);
    CHECK(uniforms[0].alignment == 4);
    CHECK(uniforms[0].size == 4);
    CHECK(uniforms[1].name == "Int");
    CHECK(uniforms[1].type == UniformType::kInt);
    CHECK(uniforms[1].alignment == 4);
    CHECK(uniforms[1].size == 4);
    CHECK(uniforms[2].name == "Float");
    CHECK(uniforms[2].type == UniformType::kFloat);
    CHECK(uniforms[2].alignment == 4);
    CHECK(uniforms[2].size == 4);
    CHECK(uniforms[3].name == "Int2");
    CHECK(uniforms[3].type == UniformType::kInt2);
    CHECK(uniforms[3].alignment == 8);
    CHECK(uniforms[3].size == 8);
    CHECK(uniforms[4].name == "Int3");
    CHECK(uniforms[4].type == UniformType::kInt3);
    CHECK(uniforms[4].alignment == 16);
    CHECK(uniforms[4].size == 12);
    CHECK(uniforms[5].name == "Int4");
    CHECK(uniforms[5].type == UniformType::kInt4);
    CHECK(uniforms[5].alignment == 16);
    CHECK(uniforms[5].size == 16);
    CHECK(uniforms[6].name == "Uint2");
    CHECK(uniforms[6].type == UniformType::kUint2);
    CHECK(uniforms[6].alignment == 8);
    CHECK(uniforms[6].size == 8);
    CHECK(uniforms[7].name == "Uint3");
    CHECK(uniforms[7].type == UniformType::kUint3);
    CHECK(uniforms[7].alignment == 16);
    CHECK(uniforms[7].size == 12);
    CHECK(uniforms[8].name == "Uint4");
    CHECK(uniforms[8].type == UniformType::kUint4);
    CHECK(uniforms[8].alignment == 16);
    CHECK(uniforms[8].size == 16);
    CHECK(uniforms[9].name == "Vec2");
    CHECK(uniforms[9].type == UniformType::kVec2);
    CHECK(uniforms[9].alignment == 8);
    CHECK(uniforms[9].size == 8);
    CHECK(uniforms[10].name == "Vec3");
    CHECK(uniforms[10].type == UniformType::kVec3);
    CHECK(uniforms[10].alignment == 16);
    CHECK(uniforms[10].size == 12);
    CHECK(uniforms[11].name == "Vec4");
    CHECK(uniforms[11].type == UniformType::kVec4);
    CHECK(uniforms[11].alignment == 16);
    CHECK(uniforms[11].size == 16);
    CHECK(uniforms[12].name == "Ivec2");
    CHECK(uniforms[12].type == UniformType::kIvec2);
    CHECK(uniforms[12].alignment == 8);
    CHECK(uniforms[12].size == 8);
    CHECK(uniforms[13].name == "Ivec3");
    CHECK(uniforms[13].type == UniformType::kIvec3);
    CHECK(uniforms[13].alignment == 16);
    CHECK(uniforms[13].size == 12);
    CHECK(uniforms[14].name == "Ivec4");
    CHECK(uniforms[14].type == UniformType::kIvec4);
    CHECK(uniforms[14].alignment == 16);
    CHECK(uniforms[14].size == 16);
    CHECK(uniforms[15].name == "Uvec2");
    CHECK(uniforms[15].type == UniformType::kUvec2);
    CHECK(uniforms[15].alignment == 8);
    CHECK(uniforms[15].size == 8);
    CHECK(uniforms[16].name == "Uvec3");
    CHECK(uniforms[16].type == UniformType::kUvec3);
    CHECK(uniforms[16].alignment == 16);
    CHECK(uniforms[16].size == 12);
    CHECK(uniforms[17].name == "Uvec4");
    CHECK(uniforms[17].type == UniformType::kUvec4);
    CHECK(uniforms[17].alignment == 16);
    CHECK(uniforms[17].size == 16);
    CHECK(uniforms[18].name == "Mat4");
    CHECK(uniforms[18].type == UniformType::kMat4);
    CHECK(uniforms[18].alignment == 16);
    CHECK(uniforms[18].size == 64);
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
