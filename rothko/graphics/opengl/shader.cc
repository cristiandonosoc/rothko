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
  handles->program_handle = prog_handle;

  /* // Get the UBO bindings. */
  /* if (!BindUBOs(shader->vert_ubos, prog_handle, handles) || */
  /*     !BindUBOs(shader->farg_ubos, prog_handle, handles)) { */
  /*   return false; */
  /* } */
  return true;
}

void FreeHandles(ShaderHandles* handles) {
  (void)handles;
  NOT_IMPLEMENTED();
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


  /* // Vertex shader. */
  /* std::string vert_source; */
  /* SubShaderParseResult vert_parse; */
  /* if (!ReadWholeFile(vert_path, &vert_source) || */
  /*     !ParseSubShader(vert_source, &vert_parse)) { */
  /*   return false; */
  /* } */

  /* // Fragment shader. */
  /* std::string frag_source; */
  /* SubShaderParseResult frag_parse; */
  /* if (!ReadWholeFile(frag_path, &frag_source) || */
  /*     !ParseSubShader(frag_source, &frag_parse)) { */
  /*   return false; */
  /* } */

  /* out->vert_source = std::move(vert_source); */
  /* out->frag_source = std::move(frag_source); */
  /* out->vert_ubos = std::move(vert_parse.ubos); */
  /* out->frag_ubos = std::move(frag_parse.ubos); */

  /* return true; */
}

}  // namespace opengl
}  // namespace rothko

