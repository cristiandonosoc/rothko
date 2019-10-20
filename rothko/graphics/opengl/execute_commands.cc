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
      case RenderCommandType::kClearFrame: continue;
      case RenderCommandType::kPushConfig: continue;
      case RenderCommandType::kPopConfig: continue;
      case RenderCommandType::kPushCamera: continue;
      case RenderCommandType::kPopCamera: continue;
      case RenderCommandType::kRenderMesh: {
        ASSERT(command.is_render_mesh());
        auto& render_mesh = command.GetRenderMesh();
        ASSERT(render_mesh.mesh);
        ASSERT(render_mesh.shader);
        ASSERT(render_mesh.primitive_type != PrimitiveType::kLast);
        ASSERT_MSG(render_mesh.mesh->vertex_type == render_mesh.shader->vertex_type,
                   "Mesh (%s): %s, Shader: (%s) %s",
                   render_mesh.mesh->name.c_str(),
                   ToString(render_mesh.mesh->vertex_type),
                   render_mesh.shader->name.c_str(),
                   ToString(render_mesh.shader->vertex_type));
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

void SetConfig(const Config& config) {
  if (config.viewport_size.width != 0 && config.viewport_size.height != 0) {
    glViewport((GLsizei)config.viewport_pos.x,
               (GLsizei)config.viewport_pos.y,
               (GLsizei)config.viewport_size.width,
               (GLsizei)config.viewport_size.height);
  }
}

void ExecutePushConfig(OpenGLRendererBackend* opengl, const PushConfig& push_config) {
  opengl->config_index++;
  ASSERT(opengl->config_index < kMaxConfigCount);

  Config* config = opengl->configs + opengl->config_index;
  config->viewport_pos = push_config.viewport_pos;
  config->viewport_size = push_config.viewport_size;
  SetConfig(*config);
}

void ExecutePopConfig(OpenGLRendererBackend* opengl) {
  // You cannot pop the first config.
  ASSERT(opengl->config_index > 0);
  opengl->config_index--;
  SetConfig(GetConfig(*opengl));
}

// Execute Push Camera -----------------------------------------------------------------------------

void ExecutePushCamera(OpenGLRendererBackend* opengl, const PushCamera& push_camera) {
  opengl->camera_index++;
  ASSERT(opengl->camera_index < kMaxCameraCount);

  CameraData* camera = opengl->cameras + opengl->camera_index;
  camera->pos = push_camera.camera_pos;
  camera->projection = push_camera.projection;
  camera->view = push_camera.view;
}

void ExecutePopCamera(OpenGLRendererBackend* opengl) {
  ASSERT(opengl->camera_index >= 0);
  opengl->camera_index--;
}

// Execute Mesh Render Actions ---------------------------------------------------------------------

void SetUniforms(const OpenGLRendererBackend& opengl, const RenderMesh& render_mesh,
                 const ShaderHandles& shader_handles) {
  const Shader* shader = render_mesh.shader;

  // Camera.
  ASSERT(opengl.camera_index >= 0);
  const CameraData& camera = GetCamera(opengl);
  if (shader_handles.camera_pos_location != -1)
    glUniform3fv(shader_handles.camera_pos_location, 1, (GLfloat*)&camera.pos);

  if (shader_handles.camera_proj_location != -1) {
    glUniformMatrix4fv(shader_handles.camera_proj_location, 1, GL_FALSE,
                       (GLfloat*)&camera.projection);
  }

  if (shader_handles.camera_view_location != -1) {
    glUniformMatrix4fv(shader_handles.camera_view_location, 1, GL_FALSE,
                       (GLfloat*)&camera.view);
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
    glBindBufferBase(GL_UNIFORM_BUFFER, ubo_binding.binding_index, ubo_binding.buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
  }
}

void SetTextures(const OpenGLRendererBackend& opengl,
                 const ShaderHandles& shader_handles,
                 const RenderMesh& render_mesh) {
  /* ASSERT(render_mesh.shader->texture_count == render_mesh.textures.size()); */
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

GLenum ToGLEnum(PrimitiveType type) {
  switch (type) {
    case PrimitiveType::kLines: return GL_LINES;
    case PrimitiveType::kLineStrip: return GL_LINE_STRIP;
    case PrimitiveType::kTriangles: return GL_TRIANGLES;
    case PrimitiveType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}

void ExecuteMeshRenderActions(const OpenGLRendererBackend& opengl, const RenderMesh& render_mesh) {
  if (render_mesh.primitive_type == PrimitiveType::kLast) {
    ERROR(OpenGL,
          "Received mesh render (%s) without primitive type", render_mesh.mesh->name.c_str());
    return;
  }

  ASSERT_MSG(render_mesh.indices_count > 0, "Received mesh render mesh command with size 0");

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
  glDrawElements(ToGLEnum(render_mesh.primitive_type),
                 render_mesh.indices_count,
                 GL_UNSIGNED_INT,
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
      case RenderCommandType::kPushConfig:
        ExecutePushConfig(opengl, command.GetPushConfig());
        break;
      case RenderCommandType::kPopConfig:
        ExecutePopConfig(opengl);
        break;
      case RenderCommandType::kPushCamera:
        ExecutePushCamera(opengl, command.GetPushCamera());
        break;
      case RenderCommandType::kPopCamera:
        ExecutePopCamera(opengl);
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
