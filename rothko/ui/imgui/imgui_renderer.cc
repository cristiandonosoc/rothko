// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/ui/imgui/imgui_renderer.h"

#include "rothko/logging/logging.h"
#include "rothko/ui/imgui/def.h"
#include "rothko/ui/imgui/imgui_shaders.h"
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
      ERROR(Imgui, "Unsupported renderer type: %s", ToString(renderer->type));
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
  imgui_mesh.vertices = std::vector<uint8_t>(KILOBYTES(512));
  imgui_mesh.vertices_count = imgui_mesh.vertices.size() / sizeof(VertexImgui);
  ASSERT(imgui_mesh.vertices_count == 26214);
  imgui_mesh.indices  = std::vector<uint8_t>(KILOBYTES(512));
  imgui_mesh.indices_count = imgui_mesh.indices.size() / sizeof(Mesh::IndexType);
  ASSERT(imgui_mesh.indices_count == 131072);

  /* imgui->mesh = CreateMesh(); */
  /* if (!RendererStageMesh(renderer, &imgui->mesh)) */
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
  texture.name = "Imgui Font";
  texture.dims = {width, height};
  texture.data = pixels;

  StageTextureConfig config = {};   // Defaults are sensible.
  if (!RendererStageTexture(config, renderer, &texture))
    return false;

  // Imgui wants a way of tracking the font texture id to relay it back to use on render time.
  imgui->font_texture = std::move(texture);
  imgui->io->Fonts->TexID = (ImTextureID)&imgui->font_texture;
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

  imgui_renderer->ubo.projection = Mat4::Identity();
  imgui_renderer->ubo.view = Mat4::Identity();

  return true;
}

// GetRenderCommand --------------------------------------------------------------------------------

PerFrameVector<RenderCommand> ImguiGetRenderCommands(ImguiRenderer* imgui_renderer) {
  ASSERT(Valid(imgui_renderer));

  ImGuiIO* io = imgui_renderer->io;
  ImDrawData* draw_data = ImGui::GetDrawData();
  draw_data->ScaleClipRects(io->DisplayFramebufferScale);

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width  = (int)(draw_data->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height = (int)(draw_data->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return {};

  // TODO(Cristian): Find out what to do about viewports.
  /* imgui_renderer->camera.viewport_p1 = {0, 0}; */
  /* imgui_renderer->camera.viewport_p2 = {fb_width, fb_height}; */

  PerFrameVector<RenderCommand> render_commands;
  ConfigRenderer config = {};
  config.viewport = {fb_width, fb_height};
  /* config.viewport = {(int)io->DisplaySize.x, (int)io->DisplaySize.y}; */
  render_commands.push_back(std::move(config));

  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  imgui_renderer->ubo.projection = Ortho(L, R, B, T);

  uint64_t base_index_offset = 0;
  uint64_t base_vertex_offset = 0;

  imgui_renderer->mesh.vertices.clear();
  imgui_renderer->mesh.vertices_count = 0;
  imgui_renderer->mesh.indices.clear();
  imgui_renderer->mesh.indices_count = 0;

  // Create the draw list.
  ImVec2 pos = draw_data->DisplayPos;
  for (int i = 0; i < draw_data->CmdListsCount; i++) {
    ImDrawList* cmd_list = draw_data->CmdLists[i];

    // Represents how much in the indices mesh buffer we're in.
    uint64_t index_offset = 0;

    // We upload the data of this draw command list.
    PushVertices(&imgui_renderer->mesh, (VertexImgui*)cmd_list->VtxBuffer.Data,
                                        cmd_list->VtxBuffer.Size);

    // Because each draw command is isolated, it's necessary to offset each
    // index by their right place in the vertex buffer.
    //
    // NOTE: Imgui doesn't have a good way of changing it's index size and compile straight out of
    //       bat. Here we do the transformation from 16-bit indexs to our 32-bit manually, but
    //       normally PushIndices should work.
    static_assert(sizeof(ImDrawIdx) == 2);
    auto& mesh = imgui_renderer->mesh;
    mesh.indices.reserve((mesh.indices_count + cmd_list->IdxBuffer.Size) * sizeof(Mesh::IndexType));
    for (int ii = 0; ii < cmd_list->IdxBuffer.Size; ii++) {
      // NOTE: |mesh.indices| is a uint8_t array, so we need to decompose the value into a series of
      //       bytes that we can append to the indices.
      Mesh::IndexType val = cmd_list->IdxBuffer[ii] + base_vertex_offset;
      uint8_t* tmp = (uint8_t*)&val;
      uint8_t* tmp_end = (uint8_t*)(&val + 1);
      mesh.indices.insert(mesh.indices.end(), tmp, tmp_end);
    }
    mesh.indices_count += cmd_list->IdxBuffer.Size;

    // This will start appending drawing data into the mesh buffer that's
    // already staged into the renderer.
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* draw_cmd = &cmd_list->CmdBuffer[cmd_i];

      // Each Imgui Draw Command is our MeshRenderCommand equivalent.
      RenderMesh render_mesh;
      render_mesh.shader = &imgui_renderer->shader;
      render_mesh.mesh = &imgui_renderer->mesh;
      /* render_mesh.textures.push_back(&imgui_renderer->font_texture); */
      render_mesh.textures.push_back((Texture*)draw_cmd->TextureId);

      render_mesh.indices_offset = base_index_offset + index_offset;
      render_mesh.indices_size = draw_cmd->ElemCount;

      render_mesh.vert_ubo_data = (uint8_t*)&imgui_renderer->ubo;
      render_mesh.blend_enabled = true;
      render_mesh.cull_faces = false;
      render_mesh.depth_test = false;
      render_mesh.scissor_test = true;

      // We check if we need to apply scissoring.
      Vec4 clip_rect;
      clip_rect.x = draw_cmd->ClipRect.x - pos.x;
      clip_rect.y = draw_cmd->ClipRect.y - pos.y;
      clip_rect.z = draw_cmd->ClipRect.z - pos.x;
      clip_rect.w = draw_cmd->ClipRect.w - pos.y;
      if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f &&
          clip_rect.w >= 0.0f) {
        render_mesh.scissor_pos.x = (int)clip_rect.x;
        render_mesh.scissor_pos.y = (int)(fb_height - clip_rect.w);
        render_mesh.scissor_size.width = (int)(clip_rect.z - clip_rect.x);
        render_mesh.scissor_size.height = (int)(clip_rect.w - clip_rect.y);
      }

      render_commands.push_back(std::move(render_mesh));

      index_offset += draw_cmd->ElemCount * sizeof(Mesh::IndexType);
    }

    // We advance the base according to how much data we added to the pool.
    base_vertex_offset += cmd_list->VtxBuffer.Size;
    /* base_index_offset += cmd_list->IdxBuffer.Size * sizeof(Mesh::IndexType); */
    base_index_offset += index_offset;
  }

  // We stage the buffers to the renderer only if there was some new information to send.
  if (!render_commands.empty()) {
    if (!RendererUploadMeshRange(imgui_renderer->renderer, &imgui_renderer->mesh))
      NOT_REACHED_MSG("Could not upload data to the renderer.");
  }

  return render_commands;
}

}  // namespace imgui
}  // namespace rothko
