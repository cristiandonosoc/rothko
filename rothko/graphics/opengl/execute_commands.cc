// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <GL/gl3w.h>

#include "rothko/graphics/graphics.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/graphics/renderer.h"
#include "rothko/logging/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {
namespace opengl {

namespace {

void ValidateRenderCommands(const PerFrameVector<RenderCommand>& commands) {
  for (auto& command : commands) {
    switch (command.type()) {
      case RenderCommandType::kClearFrame: ASSERT(command.is_clear_frame()); continue;
      case RenderCommandType::kConfigRenderer: ASSERT(command.is_config_renderer()); continue;
      case RenderCommandType::kPushCamera: continue;
      case RenderCommandType::kRenderMesh: {
        ASSERT(command.is_render_mesh());
        auto& render_mesh = command.GetRenderMesh();
        ASSERT(render_mesh.mesh);
        ASSERT(render_mesh.shader);
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

void SetRenderCommandConfig(const RenderMesh& render_mesh) {
  if (render_mesh.blend_enabled) {
    glEnable(GL_BLEND);

    // TODO(Cristian): Have a way of setting the blend function!!!!!
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }

  SET_GL_CONFIG(render_mesh.cull_faces, GL_CULL_FACE);
  SET_GL_CONFIG(render_mesh.depth_test, GL_DEPTH_TEST);
  SET_GL_CONFIG(render_mesh.scissor_test, GL_SCISSOR_TEST);
}

#define RED(c) ((float)((c >> 24) & 0xff) / 255.0f)
#define GREEN(c) ((float)((c >> 16) & 0xff) / 255.0f)
#define BLUE(c) ((float)((c >> 8) & 0xff) / 255.0f)

// Clear Frame -------------------------------------------------------------------------------------

void ExecuteClearRenderAction(const ClearFrame& clear) {
  if (!clear.clear_color && !clear.clear_depth)
    return;

  GLbitfield clear_mask = 0;
  if (clear.clear_color) {
    glClearColor(RED(clear.color), GREEN(clear.color), BLUE(clear.color), 1.0f);
    clear_mask |= GL_COLOR_BUFFER_BIT;
  }

  if (clear.clear_depth)
    clear_mask |= GL_DEPTH_BUFFER_BIT;

  glClear(clear_mask);
}

// Execute Config Renderer -------------------------------------------------------------------------

void ExecuteConfigRendererAction(const ConfigRenderer& config) {
  if (config.viewport_size.width != 0 && config.viewport_size.height != 0) {
    glViewport((GLsizei)config.viewport_base.x, (GLsizei)config.viewport_base.y,
               (GLsizei)config.viewport_size.width, (GLsizei)config.viewport_size.height);
  }
}

// Execute Push Camera -----------------------------------------------------------------------------

void ExecutePushCamera(OpenGLRendererBackend* opengl,
                       const PushCamera& push_camera) {
  opengl->camera_pos = push_camera.camera_pos;
  opengl->camera_projection = push_camera.projection;
  opengl->camera_view = push_camera.view;
}

// Execute Mesh Render Actions ---------------------------------------------------------------------

void SetUniforms(const OpenGLRendererBackend& opengl, const RenderMesh& render_mesh,
                 const ShaderHandles& shader_handles) {
  const Shader* shader = render_mesh.shader;

  // Camera.
  if (shader_handles.camera_pos_location != -1)
    glUniform3fv(shader_handles.camera_pos_location, 1, (GLfloat*)&opengl.camera_pos);

  if (shader_handles.camera_proj_location != -1) {
    glUniformMatrix4fv(shader_handles.camera_proj_location, 1, GL_FALSE,
                       (GLfloat*)&opengl.camera_projection);
  }

  if (shader_handles.camera_view_location != -1) {
    glUniformMatrix4fv(shader_handles.camera_view_location, 1, GL_FALSE,
                       (GLfloat*)&opengl.camera_view);
  }

  // Vertex UBOs.
  if (shader->vert_ubo_size > 0) {
    auto& ubo_binding = shader_handles.vert_ubo;

    ASSERT(ubo_binding.binding_index >= 0);
    ASSERT(ubo_binding.buffer_handle > 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, shader->vert_ubo_size, render_mesh.vert_ubo_data,
                 GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, ubo_binding.binding_index, ubo_binding.buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
  }

  // Fragment UBOs.
  if (shader->frag_ubo_size > 0) {
    auto& ubo_binding = shader_handles.frag_ubo;

    ASSERT(ubo_binding.binding_index >= 0);
    ASSERT(ubo_binding.buffer_handle > 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, shader->frag_ubo_size, render_mesh.frag_ubo_data,
                 GL_STREAM_DRAW);
    //glBindBufferBase(GL_UNIFORM_BUFFER, ubo_binding.binding_index, ubo_binding.buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
  }
}

void SetTextures(const OpenGLRendererBackend& opengl,
                 const ShaderHandles& shader_handles,
                 const RenderMesh& render_mesh) {
  ASSERT(render_mesh.shader->texture_count == render_mesh.textures.size());
  for (size_t i = 0; i < render_mesh.textures.size(); i++) {
    Texture* texture = render_mesh.textures[i];
    auto tex_it = opengl.loaded_textures.find(texture->uuid.value);
    ASSERT(tex_it != opengl.loaded_textures.end());
    auto& tex_handles = tex_it->second;

    uint32_t tex_handle = tex_handles.tex_handle;
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glUniform1i(shader_handles.texture_handles[i], i);
  }
}

void ExecuteMeshRenderActions(const OpenGLRendererBackend& opengl, const RenderMesh& render_mesh) {
  if (render_mesh.indices_size == 0) {
    ERROR(OpenGL, "Received mesh render mesh comman with size 0");
    return;
  }

  auto shader_it = opengl.loaded_shaders.find(render_mesh.shader->uuid.value);
  ASSERT(shader_it != opengl.loaded_shaders.end());
  const ShaderHandles& shader_handles = shader_it->second;

  // Setup the render command.
  glUseProgram(shader_handles.program);
  SetRenderCommandConfig(render_mesh);

  auto mesh_it = opengl.loaded_meshes.find(render_mesh.mesh->uuid.value);
  ASSERT(mesh_it != opengl.loaded_meshes.end());

  const MeshHandles& mesh_handles = mesh_it->second;
  SetUniforms(opengl, render_mesh, shader_handles);

  SetTextures(opengl, shader_handles, render_mesh);

  // Scissoring.
  if (render_mesh.scissor_test &&
      render_mesh.scissor_size.width != 0 && render_mesh.scissor_size.height != 0) {
    glScissor(render_mesh.scissor_pos.x, render_mesh.scissor_pos.y,
              render_mesh.scissor_size.width, render_mesh.scissor_size.height);
  }

  glBindVertexArray(mesh_handles.vao);
  glDrawElements(GL_TRIANGLES, render_mesh.indices_size, GL_UNSIGNED_INT,
                 (void*)(uint64_t)render_mesh.indices_offset);

  glBindVertexArray(NULL);
  glUseProgram(NULL);
}

}  // namespace

}  // namespace opengl

using namespace opengl;

void RendererExecuteCommands(Renderer*,
                             const PerFrameVector<RenderCommand>& commands) {
  OpenGLRendererBackend* opengl = GetOpenGL();

#if DEBUG_MODE
  ValidateRenderCommands(commands);
#endif

  for (auto& command : commands) {
    switch (command.type()) {
      case RenderCommandType::kClearFrame:
        ExecuteClearRenderAction(command.GetClearFrame());
        break;
      case RenderCommandType::kConfigRenderer:
        ExecuteConfigRendererAction(command.GetConfigRenderer());
        break;
      case RenderCommandType::kPushCamera:
        ExecutePushCamera(opengl, command.GetPushCamera());
        break;
      case RenderCommandType::kRenderMesh:
        ExecuteMeshRenderActions(*opengl, command.GetRenderMesh());
        break;
      case RenderCommandType::kLast:
        NOT_REACHED();
    }
  }
}

}  // namespace rothko
