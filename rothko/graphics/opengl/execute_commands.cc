// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/execute_commands.h"

#include <GL/gl3w.h>

#include "rothko/graphics/graphics.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/logging/logging.h"
#include "rothko/utils/macros.h"

namespace rothko {
namespace opengl {

namespace {

void ValidateRenderCommands(const PerFrameVector<RenderCommand>& commands) {
  for (auto& command : commands) {
    switch (command.type()) {
      case RenderCommandType::kClear: ASSERT(command.is_clear_frame()); continue;
      case RenderCommandType::kMesh: {
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

// Execute Mesh Render Actions -------------------------------------------------

void SetUniforms(const RenderMesh& render_mesh, const ShaderHandles& shader_handles) {
  // Vertex UBOs.
  if (Valid(render_mesh.shader->vert_ubo)) {
    auto& ubo = render_mesh.shader->vert_ubo;
    auto& ubo_binding = shader_handles.vert_ubo;

    ASSERT(ubo_binding.binding_index >= 0);
    ASSERT(ubo_binding.buffer_handle > 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, ubo.size, render_mesh.vert_ubo_data, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, ubo_binding.binding_index, ubo_binding.buffer_handle);
  }

  // Fragment UBOs.
  if (Valid(render_mesh.shader->frag_ubo)) {
    LOG(DEBUG, "SET FRAG UBO!");
    auto& ubo = render_mesh.shader->frag_ubo;
    auto& ubo_binding = shader_handles.frag_ubo;

    ASSERT(ubo_binding.binding_index >= 0);
    ASSERT(ubo_binding.buffer_handle > 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_binding.buffer_handle);
    glBufferData(GL_UNIFORM_BUFFER, ubo.size, render_mesh.frag_ubo_data, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, ubo_binding.binding_index, ubo_binding.buffer_handle);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, NULL);
}

void SetTextures(const OpenGLRendererBackend& opengl, const RenderMesh& render_mesh) {
  for (size_t i = 0; i < render_mesh.textures.size(); i++) {
    Texture* texture = render_mesh.textures[i];
    auto tex_it = opengl.loaded_textures.find(texture->uuid.value);
    ASSERT(tex_it != opengl.loaded_textures.end());

    uint32_t tex_handle = tex_it->second.tex_handle;
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
  }

}

void ExecuteMeshRenderActions(const OpenGLRendererBackend& opengl, const RenderMesh& render_mesh) {
  auto shader_it = opengl.loaded_shaders.find(render_mesh.shader->uuid.value);
  ASSERT(shader_it != opengl.loaded_shaders.end());
  const ShaderHandles& shader_handles = shader_it->second;

  /* LOG(DEBUG, "Using program %s: %u", render_mesh.shader->name.c_str(), shader_handles.program); */

  LOG(DEBUG, "---------------------------------------");
  LOG(DEBUG, "MESH ACTION: \n%s", ToString(render_mesh).c_str());
  LOG(DEBUG, "INDEX SIZE: %u", render_mesh.indices_size);

  // Setup the render command.
  glUseProgram(shader_handles.program);
  SetRenderCommandConfig(render_mesh);

  if (render_mesh.indices_size == 0) {
    LOG(WARNING, "Received mesh render mesh comman with size 0");
    return;
  }

  auto mesh_it = opengl.loaded_meshes.find(render_mesh.mesh->uuid.value);
  ASSERT(mesh_it != opengl.loaded_meshes.end());

  const MeshHandles& mesh_handles = mesh_it->second;

  LOG(DEBUG, "Setting VAO %u", mesh_handles.vao);
  glBindVertexArray(mesh_handles.vao);

  SetUniforms(render_mesh, shader_handles);
  SetTextures(opengl, render_mesh);

  // Scissoring.
  if (render_mesh.scissor_size.width != 0 && render_mesh.scissor_size.height != 0) {
    glScissor(render_mesh.scissor_pos.x, render_mesh.scissor_pos.y,
              render_mesh.scissor_size.width, render_mesh.scissor_size.height);
  }

  LOG(DEBUG, "Index size: %u, offset: %u", render_mesh.indices_size, render_mesh.indices_offset);

  glDrawElements(GL_TRIANGLES, render_mesh.indices_size, GL_UNSIGNED_INT,
                 (void*)(uint64_t)render_mesh.indices_offset);

  glBindVertexArray(NULL);
}

}  // namespace

void OpenGLExecuteCommands(const PerFrameVector<RenderCommand>& commands,
                           OpenGLRendererBackend* opengl) {
#if DEBUG_MODE
  ValidateRenderCommands(commands);
#endif

  for (auto& command : commands) {
    switch (command.type()) {
      case RenderCommandType::kClear:
        ExecuteClearRenderAction(command.GetClearFrame());
        break;
      case RenderCommandType::kMesh:
        ExecuteMeshRenderActions(*opengl, command.GetRenderMesh());
        break;
      case RenderCommandType::kLast:
        NOT_REACHED();
    }

    glUseProgram(NULL);
  }
}

}  // namespace opengl
}  // namespace rothko
