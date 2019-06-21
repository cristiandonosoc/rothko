// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/shader.h"

#include <GL/gl3w.h>

#include <atomic>
#include <optional>
#include <vector>

#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/graphics/opengl/utils.h"
#include "rothko/utils/defer.h"
#include "rothko/utils/file.h"

namespace rothko {
namespace opengl {

namespace {

std::atomic<uint32_t> kNextShaderUUID = 1;
uint32_t GetNextShaderUUID() {
  uint32_t id = kNextShaderUUID++;
  ASSERT(id < UINT32_MAX);
  return id;
}

}  // namespace

// Stage Shader ----------------------------------------------------------------

namespace {

uint32_t CompileShader(Shader* shader, const char* src, GLenum shader_kind) {
  uint32_t handle = glCreateShader(shader_kind);
  if (!handle) {
    return 0;
  }

  // Compile the shader source.
  const GLchar* gl_src = src;
  glShaderSource(handle, 1, &gl_src, 0);
  glCompileShader(handle);

  GLint success = 0;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetShaderInfoLog(handle, sizeof(log), 0, log);
    glDeleteShader(handle);
    LOG(ERROR, "Shader %s error %s: %s", shader->name.c_str(),
                                         ToString(shader_kind),
                                         log);
    return 0;
  }

  return handle;
}

uint32_t CompileVertShader(Shader* shader) {
  return CompileShader(shader, shader->vert_src.c_str(), GL_VERTEX_SHADER);
}

uint32_t CompileFragShader(Shader* shader) {
  return CompileShader(shader, shader->frag_src.c_str(), GL_FRAGMENT_SHADER);
}

// Returns the program handle or 0.
uint32_t LinkProgram(uint32_t vert_handle, uint32_t frag_handle) {
  auto cleanup = Defer([vert_handle, frag_handle]() {
    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);
  });

  uint32_t prog_handle = glCreateProgram();
  if (prog_handle == 0) {
    LOG(ERROR, "glCreateProgram: could not allocate a program");
    return 0;
  }

  // Link 'em.
  glAttachShader(prog_handle, vert_handle);
  glAttachShader(prog_handle, frag_handle);
  glLinkProgram(prog_handle);
  glDeleteShader(vert_handle);
  glDeleteShader(frag_handle);

  GLint success = 0;
  glGetProgramiv(prog_handle, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetProgramInfoLog(prog_handle, sizeof(log), 0, log);
    LOG(ERROR, "Could not link shader: %s", log);
    return 0;
  }

  return prog_handle;
}

bool BindUBOs(const std::vector<UniformBufferObject>& ubos,
              uint32_t prog_handle,
              std::vector<UBOBinding>* bindings) {
  bindings->clear();
  bindings->reserve(ubos.size());
  uint32_t current_binding = 0;
  for (auto& ubo : ubos) {

    // Obtain the block index.
    uint32_t index = glGetUniformBlockIndex(prog_handle, ubo.name.c_str());
    if (index == GL_INVALID_INDEX) {
      LOG(ERROR, "Could not find UBO index for %s", ubo.name.c_str());
      return false;
    }

    glUniformBlockBinding(prog_handle, index, current_binding);

    // Generate the buffer that will hold the uniforms.
    uint32_t buffer_handle = 0;
    glGenBuffers(1, &buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer_handle);
    ASSERT(ubo.size > 0);
    glBufferData(GL_UNIFORM_BUFFER, ubo.size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);

    // Store the binding data.
    UBOBinding binding;
    binding.binding_index = current_binding;
    binding.buffer_handle = buffer_handle;

    bindings->push_back(std::move(binding));

    current_binding++;
  }

  return true;
}

bool UploadShader(Shader* shader, ShaderHandles* handles) {
  *handles = {};

  // Compile the shaders and and program.
  uint32_t vert_handle = CompileVertShader(shader);
  uint32_t frag_handle = CompileFragShader(shader);
  if (vert_handle == 0 || frag_handle == 0) {
    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);
    return false;
  }

  // Link the program.
  // Will free |vert_handle| and |frag_handle|.
  uint32_t prog_handle = LinkProgram(vert_handle, frag_handle);
  if (prog_handle == 0)
    return false;
  handles->program = prog_handle;

  if (!BindUBOs(shader->vert_ubos, prog_handle, &handles->vert_ubos) ||
      !BindUBOs(shader->frag_ubos, prog_handle, &handles->frag_ubos)) {
    return false;
  }

  return true;
}

void FreeHandles(ShaderHandles* handles) {
  glDeleteProgram(handles->program);

  for (auto& ubo : handles->vert_ubos) {
    glDeleteBuffers(1, &ubo.buffer_handle);
  }

  for (auto& ubo : handles->frag_ubos) {
    glDeleteBuffers(1, &ubo.buffer_handle);
  }
}

}  // namespace

bool OpenGLStageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  ASSERT(Loaded(*shader));

  uint32_t uuid = GetNextShaderUUID();
  auto it = opengl->loaded_shaders.find(uuid);
  if (it != opengl->loaded_shaders.end()) {
    LOG(ERROR, "Shader %s is already loaded.", shader->name.c_str());
    return false;
  }

  ShaderHandles handles;
  if (!UploadShader(shader, &handles)) {
    FreeHandles(&handles);
    return false;
  }

  opengl->loaded_shaders[uuid] = std::move(handles);
  shader->uuid = uuid;

  return true;
}

// Unstage Shader --------------------------------------------------------------

void OpenGLUnstageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  uint32_t uuid = shader->uuid.value;
  LOG(DEBUG, "Unstaging shader %s (uuid %u).", shader->name.c_str(), uuid);
  auto it = opengl->loaded_shaders.find(uuid);
  ASSERT(it != opengl->loaded_shaders.end());

  FreeHandles(&it->second);
  opengl->loaded_shaders.erase(it);
  shader->uuid.clear();
}

}  // namespace opengl
}  // namespace rothko

