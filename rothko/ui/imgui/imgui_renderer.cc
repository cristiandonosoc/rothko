// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/ui/imgui/imgui_renderer.h"

#include "rothko/ui/imgui/def.h"
#include "rothko/ui/imgui/imgui_shaders.h"
#include "rothko/utils/logging.h"
#include "rothko/utils/types.h"

namespace rothko {
namespace imgui {

// Init --------------------------------------------------------------------------------------------

namespace {

bool CreateShader(Renderer* renderer, ImguiRenderer* imgui) {
  Shader shader;
  switch (renderer->type) {
    case RendererType::kOpenGL:
      shader = GetOpenGLImguiShader();
      break;
    case RendererType::kLast:
      LOG(ERROR, "Unsupported renderer type: %s", ToString(renderer->type));
      return false;
  }

  if (!Loaded(shader) || !RendererStageShader(renderer, &shader))
    return false;

  RemoveSources(&shader);
  imgui->shader = std::move(shader);
  return true;
}

bool CreateMesh(Renderer* renderer, ImguiRenderer* imgui) {
  // Create a Mesh for creating a buffer.
  Mesh imgui_mesh;
  imgui_mesh.name = "Imgui Mesh";
  imgui_mesh.vertex_type = VertexType::kImgui;

  // A imgui vertex is 20 bytes. An index is 4 bytes.
  // 512 kb / 20 = 26214 vertices.
  // 512 kb / 4 = 131072 indices.
  //
  // We reserve this size when staging the mesh, as we're going to re-upload pieces of this buffer
  // each time, and we don't want to be re-allocating the buffer each time.
  imgui_mesh.vertices = std::vector<uint8_t>(KILOBYTES(512) / sizeof(VertexImgui));
  imgui_mesh.indices  = std::vector<uint8_t>(KILOBYTES(512) / sizeof(Mesh::IndexType));

  if (!RendererStageMesh(renderer, &imgui_mesh))
    return false;

  imgui->mesh = std::move(imgui_mesh);
  return true;
}

bool CreateFontTexture(Renderer* renderer, ImguiRenderer* imgui) {
  ASSERT(imgui->io);

  // IMGUI AUTHOR NOTE:
  // Load as RGBA 32-bits (75% of the memory is wasted, but default font is
  // so small) because it is more likely to be compatible with user's existing
  // shaders. If your ImTextureId represent a higher-level concept than just a
  // GL texture id, consider calling GetTexDataAsAlpha8() instead to save on
  // GPU memory.
  uint8_t* pixels;
  int width, height;
  imgui->io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  Texture texture;
  texture.dims = {width, height};
  texture.data = pixels;

  StageTextureConfig config = {};   // Defaults are sensible.
  if (!RendererStageTexture(config, renderer, &texture))
    return false;

  // Imgui wants a way of tracking the font texture id to relay it back to use
  // on render time.
  imgui->io->Fonts->TexID = (ImTextureID)(uintptr_t)texture.uuid.value;
  imgui->font_texture = std::move(texture);
  return true;
}

}  // namespace

bool InitImguiRenderer(ImguiRenderer* imgui_renderer, Renderer* renderer, ImGuiIO* io) {
  ASSERT(!Valid(imgui_renderer));

  imgui_renderer->io = io;
  if (!CreateShader(renderer, imgui_renderer) ||
      !CreateMesh(renderer, imgui_renderer) ||
      !CreateFontTexture(renderer, imgui_renderer)) {
    return false;
  }

  imgui_renderer->renderer = renderer;

  return true;
}

// GetRenderCommand --------------------------------------------------------------------------------

RenderCommand ImguiGetRenderCommand(ImguiRenderer* imgui_renderer) {
  ASSERT(Valid(imgui_renderer));

  ImGuiIO* io = imgui_renderer->io;
  ImDrawData* draw_data = ImGui::GetDrawData();

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width  = (int)(draw_data->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height = (int)(draw_data->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return {};

  imgui_renderer->camera.viewport_p1 = {0, 0};
  imgui_renderer->camera.viewport_p2 = {fb_width, fb_height};

  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  imgui_renderer->camera.projection = glm::ortho(L, R, B, T);

  // Each Imgui command list is considered "isolated" from the other, starting
  // they're index base from 0. In our renderer we chain them together so we
  // need to keep track on where each command starts in our buffers.
  uint64_t base_index_offset = 0;
  uint64_t base_vertex_offset = 0;
  uint64_t total_size = 0;

  std::vector<uint32_t> index_data;

  auto mesh_actions = CreateList<MeshRenderAction>(KILOBYTES(1));

  // Create the draw list.
  ImVec2 pos = draw_data->DisplayPos;
  for (int i = 0; i < draw_data->CmdListsCount; i++) {
    SCOPE_LOCATION() << "Entry " << i;

    ImDrawList* cmd_list = draw_data->CmdLists[i];

    // Represents how much in the indices mesh buffer we're in.
    uint64_t index_offset = 0;

    // We upload the data of this draw command list.
    PushVertices(&imgui_renderer->mesh,
                 cmd_list->VtxBuffer.Data,
                 cmd_list->VtxBuffer.Size);

    // Because each draw command is isolated, it's necessary to offset each
    // index by their right place in the vertex buffer.
    PushIndicesWithOffset(&imgui_renderer->mesh,
                          cmd_list->IdxBuffer.Data,
                          cmd_list->IdxBuffer.Size,
                          base_vertex_offset);

    for (uint32_t index : cmd_list->IdxBuffer) {
      index_data.push_back(index + base_index_offset / 4);
    }

    // This will start appending drawing data into the mesh buffer that's
    // already staged into the renderer.
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* draw_cmd = &cmd_list->CmdBuffer[cmd_i];

      // Each Imgui Draw Command is our MeshRenderCommand equivalent.
      MeshRenderAction render_action;
      render_action.mesh = &imgui_renderer->mesh;
      render_action.textures = &imgui_renderer->font_texture;

      IndexRange range = 0;
      range = PushOffset(range, base_index_offset + index_offset);
      range = PushSize(range, draw_cmd->ElemCount);
      total_size += GetSize(range);
      render_action.index_range = range;

      // We check if we need to apply scissoring.
      Vec4 clip_rect;
      clip_rect.x = draw_cmd->ClipRect.x - pos.x;
      clip_rect.y = draw_cmd->ClipRect.y - pos.y;
      clip_rect.z = draw_cmd->ClipRect.z - pos.x;
      clip_rect.w = draw_cmd->ClipRect.w - pos.y;
      if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
          clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
        render_action.scissor.x = (int)clip_rect.x;
        render_action.scissor.y = (int)(fb_height - clip_rect.w);
        render_action.scissor.z = (int)(clip_rect.z - clip_rect.x);
        render_action.scissor.w = (int)(clip_rect.w - clip_rect.y);
      }

      // We push the render action into our pool.
      Push(&mesh_actions, std::move(render_action));

      index_offset += draw_cmd->ElemCount * sizeof(uint32_t);
    }

    // We advance the base according to how much data we added to the pool.
    base_vertex_offset += cmd_list->VtxBuffer.Size;
    base_index_offset += cmd_list->IdxBuffer.Size * sizeof(uint32_t);

    // We stage the buffers to the renderer.
    if (!RendererUploadMeshRange(imgui_renderer->renderer,
                                 &imgui_renderer->mesh)) {
      NOT_REACHED() << "Could not upload data to the renderer.";
    }
  }

  RenderCommand render_command;
  render_command.name = "Imgui";
  render_command.type = RenderCommandType::kMesh;
  render_command.config.blend_enabled = true;
  render_command.config.cull_faces = false;
  render_command.config.depth_test = false;
  render_command.config.scissor_test = true;
  render_command.camera = &imgui_renderer->camera;
  render_command.shader = &imgui_renderer->shader;
  render_command.mesh_actions = std::move(mesh_actions);

  return render_command;
}






}  // namespace imgui
}  // namespace rothko
