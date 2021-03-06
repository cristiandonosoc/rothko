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
      case RenderCommandType::kNop: continue;
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
        ASSERT_MSG(render_mesh.mesh->vertex_type == render_mesh.shader->config.vertex_type,
                   "Mesh (%s): %s, Shader: (%s) %s",
                   render_mesh.mesh->name.c_str(),
                   ToString(render_mesh.mesh->vertex_type),
                   render_mesh.shader->config.name.c_str(),
                   ToString(render_mesh.shader->config.vertex_type));
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
  if (GetBlendEnabled(render_mesh.flags)) {
    glEnable(GL_BLEND);

    // TODO(Cristian): Have a way of setting the blend function!!!!!
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }

  if (GetWireframeMode(render_mesh.flags)) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  SET_GL_CONFIG(GetCullFaces(render_mesh.flags), GL_CULL_FACE);
  /* glDepthFunc(GL_LESS); */
  SET_GL_CONFIG(GetDepthTest(render_mesh.flags), GL_DEPTH_TEST);
  if (GetDepthMask(render_mesh.flags)) {
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
  }
  SET_GL_CONFIG(GetScissorTest(render_mesh.flags), GL_SCISSOR_TEST);
}

#define RED(c) ((float)((c >> 24) & 0xff) / 255.0f)
#define GREEN(c) ((float)((c >> 16) & 0xff) / 255.0f)
#define BLUE(c) ((float)((c >> 8) & 0xff) / 255.0f)

// Clear Frame -------------------------------------------------------------------------------------

void ExecuteClearRenderAction(const ClearFrame& clear) {
  bool clear_color = GetClearColor(clear.flags);
  bool clear_depth = GetClearDepth(clear.flags);
  if (!clear_color && !clear_depth)
    return;

  GLbitfield clear_mask = 0;
  if (clear_color) {
    glClearColor(RED(clear.color), GREEN(clear.color), BLUE(clear.color), 1.0f);
    clear_mask |= GL_COLOR_BUFFER_BIT;
  }

  if (clear_depth)
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

  // UBOs.
  for (uint32_t i = 0; i < std::size(shader->config.ubos); i++) {
    auto& binding = shader_handles.ubos[i];
    if (binding.binding_index < 0)
      continue;

    auto& ubo = shader->config.ubos[i];
    ASSERT(binding.buffer_handle > 0);
    ASSERT(render_mesh.ubo_data[i]);

    glBindBuffer(GL_UNIFORM_BUFFER, binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, ubo.size, render_mesh.ubo_data[i], GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding.binding_index, binding.buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
  }
}

void SetTextures(const OpenGLRendererBackend& opengl,
                 const ShaderHandles& shader_handles,
                 const RenderMesh& render_mesh) {
  /* ASSERT(render_mesh.shader->texture_count == render_mesh.textures.size()); */
  for (size_t i = 0; i < render_mesh.textures.size(); i++) {
    Texture* texture = render_mesh.textures[i];
    const TextureHandles* tex_handles = nullptr;
    if (!texture) {
      auto white_it = opengl.loaded_textures.find(opengl.white_texture->uuid.value);
      ASSERT(white_it != opengl.loaded_textures.end());
      tex_handles = &white_it->second;
    } else {
      auto tex_it = opengl.loaded_textures.find(texture->uuid.value);
      ASSERT(tex_it != opengl.loaded_textures.end());
      tex_handles = &tex_it->second;
    }

    uint32_t tex_handle = tex_handles->tex_handle;
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

  auto mesh_it = opengl.loaded_meshes.find(render_mesh.mesh->id);
  ASSERT(mesh_it != opengl.loaded_meshes.end());

  const MeshHandles& mesh_handles = mesh_it->second;
  SetUniforms(opengl, render_mesh, shader_handles);
  SetTextures(opengl, shader_handles, render_mesh);

  // Scissoring.
  if (GetScissorTest(render_mesh.flags) &&
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
      case RenderCommandType::kNop:
        continue;
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
