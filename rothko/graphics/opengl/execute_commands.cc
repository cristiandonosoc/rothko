// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/execute_commands.h"

#include <GL/gl3w.h>

#include "rothko/graphics/graphics.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {
namespace opengl {

namespace {

void ValidateRenderCommands(const PerFrameVector<RenderCommand>& commands) {
  for (auto& command : commands) {
    switch (command.type) {
      case RenderCommandType::kClear: ASSERT(command.is_clear_action()); continue;
      case RenderCommandType::kMesh: {
        ASSERT(command.is_mesh_actions());
        ASSERT(command.shader);
        for (auto& action : command.MeshActions()) {
          ASSERT(action.mesh);
        }
        continue;
      }
      case RenderCommandType::kLast: break;
    }

    NOT_REACHED();
  }
}

#define SET_GL_CONFIG(flag, gl_name) \
  if (flag) {                        \
    glEnable(gl_name);               \
  } else {                           \
    glDisable(gl_name);              \
  }

void SetRenderCommandConfig(const RenderCommand& command) {
  if (command.blend_enabled) {
    glEnable(GL_BLEND);

    // TODO(Cristian): Have a way of setting the blend function!!!!!
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }

  SET_GL_CONFIG(command.cull_faces, GL_CULL_FACE);
  SET_GL_CONFIG(command.depth_test, GL_DEPTH_TEST);
  SET_GL_CONFIG(command.scissor_test, GL_SCISSOR_TEST);
}

void ExecuteClearRenderAction(const ClearRenderAction& clear) {
  if (!clear.clear_color && !clear.clear_depth)
    return;

  GLbitfield clear_mask = 0;
  if (clear.clear_color) {
    glClearColor(clear.color.r, clear.color.g, clear.color.b, 1.0f);
    clear_mask |= GL_COLOR_BUFFER_BIT;
  }

  if (clear.clear_depth)
    clear_mask |= GL_DEPTH_BUFFER_BIT;

  glClear(clear_mask);
}

// Execute Mesh Render Actions -------------------------------------------------

void SetUniforms(const Shader& shader, const ShaderHandles& shader_handles,
                 const MeshRenderAction& action) {
  ASSERT(shader.vert_ubos.size() == action.vert_ubos.size());
  ASSERT(shader.vert_ubos.size() == shader_handles.vert_ubos.size());

  for (size_t i = 0; i < shader.vert_ubos.size(); i++) {
    auto& ubo = shader.vert_ubos[i];
    auto& ubo_binding = shader_handles.vert_ubos[i];

    ASSERT(ubo_binding.binding_index >= 0);
    ASSERT(ubo_binding.buffer_handle > 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, ubo.size, action.vert_ubos[i], GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
  }

  ASSERT(shader.frag_ubos.size() == action.frag_ubos.size());
  ASSERT(shader.frag_ubos.size() == shader_handles.frag_ubos.size());

  for (size_t i = 0; i < shader.frag_ubos.size(); i++) {
    auto& ubo = shader.frag_ubos[i];
    auto& ubo_binding = shader_handles.frag_ubos[i];

    ASSERT(ubo_binding.binding_index >= 0);
    ASSERT(ubo_binding.buffer_handle > 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, ubo.size, action.frag_ubos[i], GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
  }
}

void SetTextures(const OpenGLRendererBackend& opengl,
                 const Shader& shader,
                 const MeshRenderAction& action) {
  if (action.textures.empty())
    return;
  (void)opengl;
  (void)shader;
  (void)action;
  NOT_IMPLEMENTED();
}

void
ExecuteMeshRenderActions(const OpenGLRendererBackend& opengl,
                         const RenderCommand& command) {
  auto shader_it = opengl.loaded_shaders.find(command.shader->uuid.value);
  ASSERT(shader_it != opengl.loaded_shaders.end());
  const ShaderHandles& shader_handles = shader_it->second;

  // Setup the render command.
  glUseProgram(shader_handles.program);
  SetRenderCommandConfig(command);

  for (const MeshRenderAction& action : command.MeshActions()) {
    if (action.indices_size == 0) {
      LOG(WARNING, "Received mesh render action with size 0");
      continue;
    }

    auto mesh_it = opengl.loaded_meshes.find(action.mesh->uuid.value);
    ASSERT(mesh_it != opengl.loaded_meshes.end());

    const MeshHandles& mesh_handles = mesh_it->second;
    glBindVertexArray(mesh_handles.vao);

    SetUniforms(*command.shader, shader_handles, action);
    SetTextures(opengl, *command.shader, action);

    // Scissoring.
    if (action.scissor_size.width != 0 && action.scissor_size.height != 0) {
      glScissor(action.scissor_pos.x, action.scissor_pos.y,
                action.scissor_size.width, action.scissor_size.height);
    }

    glDrawElements(GL_TRIANGLES, action.indices_size,
                                 GL_UNSIGNED_INT,
                                 (void*)(uint64_t)action.indices_offset);
  }

  glBindVertexArray(NULL);
}

}  // namespace

void OpenGLExecuteCommands(const PerFrameVector<RenderCommand>& commands,
                           OpenGLRendererBackend* opengl) {
#if DEBUG_MODE
  ValidateRenderCommands(commands);
#endif

  for (auto& command : commands) {
    switch (command.type) {
      case RenderCommandType::kClear:
        ExecuteClearRenderAction(command.ClearAction());
        break;
      case RenderCommandType::kMesh:
        ExecuteMeshRenderActions(*opengl, command);
        break;
      case RenderCommandType::kLast:
        NOT_REACHED();
    }

    glUseProgram(NULL);
  }
}

}  // namespace opengl
}  // namespace rothko
