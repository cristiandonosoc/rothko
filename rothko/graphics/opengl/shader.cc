// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/shader.h"

#include <GL/gl3w.h>

#include <atomic>
#include <optional>
#include <vector>

#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/graphics/opengl/utils.h"
#include "rothko/logging/logging.h"
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
    ERROR(OpenGL, "Shader %s error %s: %s", shader->name.c_str(), ToString(shader_kind), log);
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
    ERROR(OpenGL, "glCreateProgram: could not allocate a program");
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
    ERROR(OpenGL, "Could not link shader: %s", log);
    return 0;
  }

  return prog_handle;
}

bool BindUBOs(const std::string& ubo_name, uint32_t ubo_size,
              uint32_t prog_handle,
              ShaderHandles::UBO* binding) {
  if (ubo_size == 0)
    return true;

  uint32_t current_binding = 0;

  // Obtain the block index.
  uint32_t index = glGetUniformBlockIndex(prog_handle, ubo_name.c_str());
  if (index == GL_INVALID_INDEX) {
    ERROR(OpenGL, "Could not find UBO index for %s", ubo_name.c_str());
    return false;
  }

  glUniformBlockBinding(prog_handle, index, current_binding);

  // Generate the buffer that will hold the uniforms.
  uint32_t buffer_handle = 0;
  glGenBuffers(1, &buffer_handle);
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_handle);
  ASSERT(ubo_size > 0);
  glBufferData(GL_UNIFORM_BUFFER, ubo_size, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, NULL);

  // Store the binding data.
  binding->binding_index = current_binding;
  binding->buffer_handle = buffer_handle;

  return true;
}

bool GetUniformLocation(uint32_t program, const char* uniform_name, int* out) {
  GLint result = glGetUniformLocation(program, uniform_name);
  if (result == GL_INVALID_VALUE || result == GL_INVALID_OPERATION || result == -1) {
    ERROR(OpenGL, "Could not get uniform %s", uniform_name);
    return false;
  }

  *out = result;
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

  // Get the camera uniform locations.
  if (!GetUniformLocation(handles->program, "proj", &handles->proj_location) ||
      !GetUniformLocation(handles->program, "view", &handles->view_location)) {
    ERROR(OpenGL, "Could not find camera uniform(s) for shader %s", shader->name.c_str());
    return false;
  }

  // Get the uniform buffer object information.
  if (!BindUBOs(shader->vert_ubo_name, shader->vert_ubo_size, prog_handle, &handles->vert_ubo) ||
      !BindUBOs(shader->frag_ubo_name, shader->frag_ubo_size, prog_handle, &handles->frag_ubo)) {
    return false;
  }

  // Get the texture positions.
  for (uint32_t i = 0; i < shader->texture_count; i++) {
    char tex_name[] = "tex%";   // % will be replaced.
    tex_name[3] = '0' + i;

    handles->texture_handles[i] = glGetUniformLocation(handles->program, tex_name);
  }

  return true;
}

void FreeHandles(ShaderHandles* handles) {
  glDeleteProgram(handles->program);
  glDeleteBuffers(1, &handles->vert_ubo.buffer_handle);
  glDeleteBuffers(1, &handles->frag_ubo.buffer_handle);
}

}  // namespace

bool OpenGLStageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  ASSERT(Loaded(*shader));

  uint32_t uuid = GetNextShaderUUID();
  auto it = opengl->loaded_shaders.find(uuid);
  if (it != opengl->loaded_shaders.end()) {
    ERROR(OpenGL, "Shader %s is already loaded.", shader->name.c_str());
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
  LOG(OpenGL, "Unstaging shader %s (uuid %u).", shader->name.c_str(), uuid);
  auto it = opengl->loaded_shaders.find(uuid);
  ASSERT(it != opengl->loaded_shaders.end());

  FreeHandles(&it->second);
  opengl->loaded_shaders.erase(it);
  shader->uuid.clear();
}

}  // namespace opengl
}  // namespace rothko

