// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/graphics/definitions.h"
#include "rothko/graphics/vertices.h"
#include "rothko/utils/clear_on_move.h"
#include "rothko/utils/macros.h"

namespace rothko {

// Useful macros for padding UBO structs to conform to layout std140.
#define FLOAT_PAD() float STRINGIFY(__pad_, __LINE__)

struct Renderer;

// Shader
//
// Rothko's shaders use certain shader features:
// Uniform Buffer Objects: Uniforms are treated as a single struct that is memcpy'ed over to the
//                         GPU. Rothko expects at most one UBO for vertex shader and at most one
//                         UBO for fragment shader. Both can have no UBO.
//
// Expected uniforms: Rothko expects certain uniforms to be defined and some other are provided
//                    as optional. In order to avoid adding them manually, you should always
//                    pass your shader source through |CreateVertSource| and  |CreateFragSource|.
//
//
// Provided uniforms: All of them are optional.
// - camera_pos: The position of the camera. Passed through the |PushCamera| command.
// - camera_proj: The projection matrix. Passed through the |PushCamera| command.
// - camera_view: The view matrix. Passed through the |PushCamera| command.

struct ShaderConfig {
  std::string name;   // Used as key, must be unique.

  VertexType vertex_type = VertexType::kLast;

  // A UniformBufferObject is a group of uniforms grouped in a struct-ish configuration within the
  // shader. The advantage of those is that they can be mapped directly from a buffer upload
  // (eg. memcpy) instead of individually through glUniform1v kind of calls.
  //
  // The name *must* match the uniform block name within the shader. Otherwise staging the shader
  // will fail. After that, on the |RenderMesh| command, you can define the UBO pointers in the same
  // order they were defined here.
  struct UBO {
    std::string name;
    uint32_t size = 0;
  };
  UBO ubos[kMaxUBOs] = {};

  uint32_t texture_count = 0;

};

struct Shader {
  RAII_CONSTRUCTORS(Shader);

  Renderer* renderer = nullptr;
  ClearOnMove<uint32_t> uuid = 0;  // Set by the renderer.

  ShaderConfig config = {};

  std::string vert_src;
  std::string frag_src;
};

inline bool Valid(const Shader& s) { return s.config.vertex_type != VertexType::kLast; }
inline bool Loaded(const Shader& s) { return !s.vert_src.empty() && !s.frag_src.empty(); }
inline bool Staged(const Shader& s) { return s.renderer && s.uuid.has_value(); }

// Rothko will add a header in which it will define some uniforms and functions that are common
// functionality to all shaders.
//
// If |header| is not provided, Rothko will append the header with the glsl version and some
// extensions. If provided, it will append that instead.
std::string CreateVertexSource(const std::string& vert_src, const char* header = nullptr);
std::string CreateFragmentSource(const std::string& frag_src, const char* header = nullptr);

bool LoadShaderSources(const std::string& vert_path,
                       const std::string& frag_path,
                       Shader* out);
void RemoveSources(Shader* s);

}  // namespace rothko
