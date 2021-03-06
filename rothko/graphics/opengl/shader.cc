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
#include "rothko/utils/strings.h"

namespace rothko {
namespace opengl {

namespace {

std::atomic<uint32_t> kNextShaderUUID = 1;
uint32_t GetNextShaderUUID() {
  uint32_t id = kNextShaderUUID++;
  ASSERT(id < UINT32_MAX);
  return id;
}

void OutputShaderForError(const std::string& source) {
  auto lines = SplitToLinesKeepEmpty(source, "\n", "\t\r");
  for (uint32_t i = 0; i < lines.size(); i++) {
    ERROR(OpenGL, "%04u: %s", i + 1, lines[i].c_str());
  }
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
    ERROR(OpenGL, "* VERT SOURCE ---------------------------------------\n");
    OutputShaderForError(shader->vert_src);
    ERROR(OpenGL, "* FRAG SOURCE ---------------------------------------\n");
    OutputShaderForError(shader->frag_src);
    ERROR(OpenGL, "---------------------------------------\n");
    ERROR(
        OpenGL, "Shader %s error %s: %s", shader->config.name.c_str(), ToString(shader_kind), log);
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

bool BindUBO(const std::string& ubo_name, uint32_t ubo_size, uint32_t prog_handle,
             ShaderHandles::UBO* binding, int* current_binding) {
  if (ubo_size == 0)
    return true;

  // Obtain the block index.
  uint32_t index = glGetUniformBlockIndex(prog_handle, ubo_name.c_str());
  if (index == GL_INVALID_INDEX) {
    ERROR(OpenGL, "Could not find UBO index for %s", ubo_name.c_str());
    return false;
  }

  glUniformBlockBinding(prog_handle, index, *current_binding);

  // Generate the buffer that will hold the uniforms.
  uint32_t buffer_handle = 0;
  glGenBuffers(1, &buffer_handle);
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_handle);
  ASSERT(ubo_size > 0);
  glBufferData(GL_UNIFORM_BUFFER, ubo_size, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, NULL);

  // Store the binding data.
  binding->binding_index = *current_binding;
  binding->buffer_handle = buffer_handle;

  *current_binding += 1;

  return true;
}

// |log| means whether to log that we didn't find the uniform. In some cases, it is valid to not
// find the uniform (like camera_pos), so we don't want to clutter the logs in that case.
bool GetUniformLocation(uint32_t program, const char* uniform_name, int* out, bool log = false) {
  GLint result = glGetUniformLocation(program, uniform_name);
  if (result == GL_INVALID_VALUE || result == GL_INVALID_OPERATION || result == -1) {
    if (log)
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
  // |camera_pos| is optional.
  GetUniformLocation(handles->program, "camera_pos", &handles->camera_pos_location);
  GetUniformLocation(handles->program, "camera_proj", &handles->camera_proj_location);
  GetUniformLocation(handles->program, "camera_view", &handles->camera_view_location);

  // Camera pos is optional.

  // Get the uniform buffer object information.
  int current_binding = 0;
  for (uint32_t i = 0; i < std::size(shader->config.ubos); i++) {
    auto& ubo = shader->config.ubos[i];
    if (!BindUBO(ubo.name, ubo.size, prog_handle, &handles->ubos[i], &current_binding))
      return false;
  }

  // Get the texture positions.
  for (uint32_t i = 0; i < shader->config.texture_count; i++) {
    char tex_name[] = "tex%";   // % will be replaced.
    tex_name[3] = '0' + i;

    handles->texture_handles[i] = glGetUniformLocation(handles->program, tex_name);
  }

  return true;
}

void FreeHandles(ShaderHandles* handles) {
  glDeleteProgram(handles->program);

  for (auto& ubo : handles->ubos) {
    glDeleteBuffers(1, &ubo.buffer_handle);
  }
}

}  // namespace

std::unique_ptr<Shader> OpenGLStageShader(OpenGLRendererBackend* opengl,
                                          const ShaderConfig& config,
                                          const std::string& vert_src,
                                          const std::string& frag_src) {
  // TODO(Cristian): Keep track by name.

  uint32_t uuid = GetNextShaderUUID();

  auto shader = std::make_unique<Shader>();
  shader->config = config;
  shader->vert_src = vert_src;
  shader->frag_src = frag_src;

  ShaderHandles handles;
  if (!UploadShader(shader.get(), &handles)) {
    FreeHandles(&handles);
    return {};
  }

  opengl->loaded_shaders[uuid] = std::move(handles);
  shader->uuid = uuid;

  return shader;
}

// Unstage Shader --------------------------------------------------------------

void OpenGLUnstageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  uint32_t uuid = shader->uuid.value;
  LOG(OpenGL, "Unstaging shader %s (uuid %u).", shader->config.name.c_str(), uuid);
  auto it = opengl->loaded_shaders.find(uuid);
  ASSERT(it != opengl->loaded_shaders.end());

  FreeHandles(&it->second);
  opengl->loaded_shaders.erase(it);
  shader->uuid.clear();
}

}  // namespace opengl
}  // namespace rothko

