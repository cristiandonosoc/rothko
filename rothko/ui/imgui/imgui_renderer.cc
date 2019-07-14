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
      LOG(ERROR, "Unsupported renderer type: %s", ToString(renderer->type));
      return false;
  }

  if (!Loaded(shader) || !RendererStageShader(renderer, &shader))
    return false;

  RemoveSources(&shader);
  imgui->shader = std::move(shader);
  return true;
}

/* struct Colors { */
/*   // abgr */
/*   /1* static constexpr uint32_t kBlack=   0x00'00'00'ff; *1/ */
/*   /1* static constexpr uint32_t kBlue=    0x00'00'ff'ff; *1/ */
/*   /1* static constexpr uint32_t kGreen =  0x00'ff'00'ff; *1/ */
/*   /1* static constexpr uint32_t kRed =    0xff'00'00'ff; *1/ */
/*   /1* static constexpr uint32_t kWhite =  0xff'ff'ff'ff; *1/ */
/*   /1* static constexpr uint32_t kTeal =   0xff'f9'f0'ea; *1/ */
/*   /1* static constexpr uint32_t kGray =   0xff'99'99'99; *1/ */

/*   /1* static constexpr uint32_t kBlack=   0xff'00'00'00; *1/ */
/*   static constexpr uint32_t kBlue=    0xff'ff'00'00; */
/*   static constexpr uint32_t kGreen =  0xff'00'ff'00; */
/*   static constexpr uint32_t kRed =    0xff'00'00'ff; */
/*   static constexpr uint32_t kWhite =  0xff'ff'ff'ff; */
/* }; */

/* Mesh CreateMesh() { */
/*   Mesh mesh = {}; */
/*   mesh.name = "IMGUI CUBEZ"; */
/*   mesh.vertex_type = VertexType::kColor; */

/*   VertexColor vertices[] = { */
/*     // X */
/*     {{-1, -1, -1}, Colors::kBlue}, */
/*     {{-1, -1,  1}, Colors::kGreen}, */
/*     {{-1,  1,  1}, Colors::kWhite}, */
/*     {{-1,  1, -1}, Colors::kRed}, */
/*     {{-1, -1, -1}, Colors::kBlue}, */
/*     {{-1, -1,  1}, Colors::kGreen}, */
/*     {{-1,  1,  1}, Colors::kWhite}, */
/*     {{-1,  1, -1}, Colors::kRed}, */

/*     // Y */
/*     {{-1, -1, -1}, Colors::kBlue}, */
/*     {{ 1, -1, -1}, Colors::kGreen}, */
/*     {{ 1, -1,  1}, Colors::kWhite}, */
/*     {{-1, -1,  1}, Colors::kRed}, */
/*     {{-1,  1, -1}, Colors::kBlue}, */
/*     {{ 1,  1, -1}, Colors::kGreen}, */
/*     {{ 1,  1,  1}, Colors::kWhite}, */
/*     {{-1,  1,  1}, Colors::kRed}, */

/*     // Z */
/*     {{-1, -1, -1}, Colors::kBlue}, */
/*     {{ 1, -1, -1}, Colors::kGreen}, */
/*     {{ 1,  1, -1}, Colors::kWhite}, */
/*     {{-1,  1, -1}, Colors::kRed}, */
/*     {{-1, -1,  1}, Colors::kBlue}, */
/*     {{ 1, -1,  1}, Colors::kGreen}, */
/*     {{ 1,  1,  1}, Colors::kWhite}, */
/*     {{-1,  1,  1}, Colors::kRed}, */
/*   }; */


/*   /1* mesh.vertex_type = VertexType::kImgui; *1/ */

/*   /1* VertexImgui vertices[] = { *1/ */
/*   /1*   // X *1/ */
/*   /1*   {{-1, -1}, {0, -1}, Colors::kBlue}, *1/ */
/*   /1*   {{-1, -1}, {0,  1}, Colors::kGreen}, *1/ */
/*   /1*   {{-1,  1}, {0,  1}, Colors::kWhite}, *1/ */
/*   /1*   {{-1,  1}, {0, -1}, Colors::kRed}, *1/ */
/*   /1*   {{-1, -1}, {0, -1}, Colors::kBlue}, *1/ */
/*   /1*   {{-1, -1}, {0,  1}, Colors::kGreen}, *1/ */
/*   /1*   {{-1,  1}, {0,  1}, Colors::kWhite}, *1/ */
/*   /1*   {{-1,  1}, {0, -1}, Colors::kRed}, *1/ */

/*   /1*   // Y *1/ */
/*   /1*   {{-1, -1}, {0, -1}, Colors::kBlue}, *1/ */
/*   /1*   {{ 1, -1}, {0, -1}, Colors::kGreen}, *1/ */
/*   /1*   {{ 1, -1}, {0,  1}, Colors::kWhite}, *1/ */
/*   /1*   {{-1, -1}, {0,  1}, Colors::kRed}, *1/ */
/*   /1*   {{-1,  1}, {0, -1}, Colors::kBlue}, *1/ */
/*   /1*   {{ 1,  1}, {0, -1}, Colors::kGreen}, *1/ */
/*   /1*   {{ 1,  1}, {0,  1}, Colors::kWhite}, *1/ */
/*   /1*   {{-1,  1}, {0,  1}, Colors::kRed}, *1/ */

/*   /1*   // Z *1/ */
/*   /1*   {{-1, -1}, {0, -1}, Colors::kBlue}, *1/ */
/*   /1*   {{ 1, -1}, {0, -1}, Colors::kGreen}, *1/ */
/*   /1*   {{ 1,  1}, {0, -1}, Colors::kWhite}, *1/ */
/*   /1*   {{-1,  1}, {0, -1}, Colors::kRed}, *1/ */
/*   /1*   {{-1, -1}, {0,  1}, Colors::kBlue}, *1/ */
/*   /1*   {{ 1, -1}, {0,  1}, Colors::kGreen}, *1/ */
/*   /1*   {{ 1,  1}, {0,  1}, Colors::kWhite}, *1/ */
/*   /1*   {{-1,  1}, {0,  1}, Colors::kRed}, *1/ */
/*   /1* }; *1/ */


/*   Mesh::IndexType indices[] = { */
/*     0, 1, 2, 2, 3, 0, */
/*     4, 5, 6, 6, 7, 4, */

/*     8, 9, 10, 10, 11, 8, */
/*     12, 13, 14, 14, 15, 12, */

/*     16, 17, 18, 18, 19, 16, */
/*     20, 21, 22, 22, 23, 20, */
/*   }; */

/*   PushVertices(&mesh, vertices, ARRAY_SIZE(vertices)); */
/*   PushIndices(&mesh, indices, ARRAY_SIZE(indices)); */

/*   ASSERT(mesh.vertices_count == 24); */
/*   ASSERT(mesh.vertices.size() == sizeof(vertices)); */

/*   ASSERT_MSG(mesh.indices_count == 36, "Count: %u", mesh.indices_count); */
/*   ASSERT(mesh.indices.size() == sizeof(indices)); */

/*   return mesh; */
/* } */

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
  imgui_mesh.indices  = std::vector<uint8_t>(KILOBYTES(512));

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

  imgui_renderer->ubo.projection = Mat4::Identity();
  imgui_renderer->ubo.view = Mat4::Identity();

  return true;
}

// GetRenderCommand --------------------------------------------------------------------------------

PerFrameVector<RenderCommand> ImguiGetRenderCommands(ImguiRenderer* imgui_renderer) {
  ASSERT(Valid(imgui_renderer));

  ImGuiIO* io = imgui_renderer->io;
  ImDrawData* draw_data = ImGui::GetDrawData();

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width  = (int)(draw_data->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height = (int)(draw_data->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return {};

  // TODO(Cristian): Find out what to do about viewports.
  /* imgui_renderer->camera.viewport_p1 = {0, 0}; */
  /* imgui_renderer->camera.viewport_p2 = {fb_width, fb_height}; */

  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  imgui_renderer->ubo.projection = Ortho(L, R, B, T);
  /* imgui_renderer->camera.projection = glm::ortho(L, R, B, T); */

  // Each Imgui command list is considered "isolated" from the other, starting
  // they're index base from 0. In our renderer we chain them together so we
  // need to keep track on where each command starts in our buffers.
  uint64_t base_index_offset = 0;
  uint64_t base_vertex_offset = 0;

  imgui_renderer->mesh.vertices.clear();
  imgui_renderer->mesh.vertices_count = 0;
  imgui_renderer->mesh.indices.clear();
  imgui_renderer->mesh.indices_count = 0;

  PerFrameVector<RenderCommand> render_commands;

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
    static_assert(sizeof(ImDrawIdx) == sizeof(Mesh::IndexType));
    PushIndices(&imgui_renderer->mesh, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size,
                base_vertex_offset);

    // This will start appending drawing data into the mesh buffer that's
    // already staged into the renderer.
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* draw_cmd = &cmd_list->CmdBuffer[cmd_i];

      // Each Imgui Draw Command is our MeshRenderCommand equivalent.
      RenderMesh render_mesh;
      render_mesh.shader = &imgui_renderer->shader;
      render_mesh.mesh = &imgui_renderer->mesh;
      render_mesh.textures.push_back(&imgui_renderer->font_texture);

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

  /* RenderCommand render_command; */
  /* render_command.name = "Imgui"; */
  /* render_command.type = RenderCommandType::kMesh; */
  /* render_command.config.blend_enabled = true; */
  /* render_command.config.cull_faces = false; */
  /* render_command.config.depth_test = false; */
  /* render_command.config.scissor_test = true; */
  /* render_command.camera = &imgui_renderer->camera; */
  /* render_command.shader = &imgui_renderer->shader; */
  /* render_command.mesh_actions = std::move(mesh_actions); */

  return render_commands;
}

}  // namespace imgui
}  // namespace rothko
