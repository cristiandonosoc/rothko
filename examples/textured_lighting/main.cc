// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>
#include <rothko/ui/imgui.h>
#include <third_party/imguizmo/ImGuizmo.h>

#include "shaders.h"

using namespace rothko;
using namespace imgui;

namespace {

template <typename UBO>
RenderMesh CreateRenderCommand(Mesh* mesh,
                               Shader* shader,
                               Texture* diffuse_map,
                               Texture* specular_map,
                               const UBO& ubo) {
  RenderMesh render_mesh = {};
  render_mesh.mesh = mesh;
  render_mesh.shader = shader;
  render_mesh.primitive_type = PrimitiveType::kTriangles;

  render_mesh.indices_count = mesh->indices.size();

  render_mesh.vert_ubo_data = (uint8_t*)&ubo.vert;
  render_mesh.frag_ubo_data = (uint8_t*)&ubo.frag;

  if (diffuse_map)
    render_mesh.textures.push_back(diffuse_map);

  if (specular_map)
    render_mesh.textures.push_back(specular_map);

  return render_mesh;
}

};  // namespace

int main() {
  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;

  ImguiContext imgui;
  if (!InitImgui(game.renderer.get(), &imgui))
    return 1;

  Shader object_shader = simple_lighting::CreateObjectShader(game.renderer.get());
  if (!Valid(object_shader))
    return 1;

  Shader light_shader = simple_lighting::CreateLightShader(game.renderer.get());
  if (!Valid(light_shader))
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  Grid grid;
  if (!Init(game.renderer.get(), &grid, "main-grid"))
    return 1;

  Mesh cube_mesh = CreateCubeMesh(VertexType::k3dNormalUV, "cube");
  if (!RendererStageMesh(game.renderer.get(), &cube_mesh))
    return 1;

  Mesh light_cube_mesh = CreateCubeMesh(VertexType::k3d, "light-cube");
  if (!RendererStageMesh(game.renderer.get(), &light_cube_mesh))
    return 1;

  // Load the texture.
  std::string diffuse_map_path = JoinPaths(
      {GetCurrentExecutableDirectory(), "..", "examples", "textured_lighting", "diffuse_map.png"});
  Texture diffuse_map = {};
  if (!STBLoadTexture(diffuse_map_path, TextureType::kRGBA, &diffuse_map) ||
      !RendererStageTexture(game.renderer.get(), &diffuse_map)) {
    return 1;
  }

  std::string specular_map_path = JoinPaths(
      {GetCurrentExecutableDirectory(), "..", "examples", "textured_lighting", "specular_map.png"});
  Texture specular_map = {};
  if (!STBLoadTexture(specular_map_path, TextureType::kRGBA, &specular_map) ||
      !RendererStageTexture(game.renderer.get(), &specular_map)) {
    return 1;
  }

  simple_lighting::LightShaderUBO light_ubo = {};
  light_ubo.vert.model = Mat4::Identity();
  light_ubo.frag.light_color = ToVec3(Color::White());

  // Create the UBOs.
  constexpr int kCubeCount = 8;
  std::vector<simple_lighting::ObjectShaderUBO> ubos;
  ubos.reserve(kCubeCount);

  constexpr int kMaxRow = 4;
  /* for (auto& [name, mat] :) { */
  for (int i = 0; i < kCubeCount; i++) {
    simple_lighting::ObjectShaderUBO ubo = {};
    /* ubo.frag.light.ambient = {0.2f, 0.2f, 0.2f}; */
    ubo.frag.light.ambient = {};
    ubo.frag.light.diffuse = {0.5f, 0.5f, 0.5f};
    ubo.frag.light.specular = {1, 1, 1};

    ubo.frag.material.specular = ToVec3(Color::White());
    ubo.frag.material.shininess = 128;

    ubos.push_back(std::move(ubo));
  }

  bool running = true;

  Vec3 light_pos = {};
  bool move_light = false;
  float light_time_delta = 0;
  (void)light_time_delta;

  bool move_cubes = true;
  float cubes_time_delta = 0;


    /* light_ubo.vert.model = Translate(light_pos); */
    /* light_ubo.vert.model *= Scale(0.1f); */

  while (running) {
    auto events = Update(&game);
    StartFrame(&imgui, &game.window, &game.time, &game.input);
    ImGuizmo::BeginFrame();

    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(&game.input, Key::kEscape)) {
      running = false;
      break;
    }

    if (KeyUpThisFrame(&game.input, Key::kSpace))
      move_light = !move_light;

    if (KeyUpThisFrame(&game.input, Key::kC))
      move_cubes = !move_cubes;

    DefaultUpdateOrbitCamera(game.input, &camera);

    auto camera_command = GetCommand(camera);


    // Update the UBOs.

    /* if (move_light) { */
    /*   light_time_delta += game.time.frame_delta; */
    /*   light_pos = Vec3(Sin(light_time_delta) * 4, 0.1f, 1); */
    /* } else { */
    /*   ImGuizmo::SetRect(0, 0, game.window.screen_size.width, game.window.screen_size.height); */
    /*   ImGuizmo::Manipulate((float*)&camera_command.view, */
    /*                        (float*)&camera_command.projection, */
    /*                        ImGuizmo::OPERATION::TRANSLATE, */
    /*                        ImGuizmo::MODE::WORLD, */
    /*                        (float*)&light_ubo.vert.model); */

    /*   // Extract pos from it. */
    /*   light_pos.x = light_ubo.vert.model.get(3, 0); */
    /*   light_pos.y = light_ubo.vert.model.get(3, 1); */
    /*   light_pos.z = light_ubo.vert.model.get(3, 2); */
    /* } */

    /* light_ubo.vert.model = Translate(light_pos); */
    /* light_ubo.vert.model *= Scale(0.1f); */
    ImGui::Begin("Matrix");
    for (int i = 0; i < 4; i++) {
      auto row = light_ubo.vert.model.row(0);
      ImGui::InputFloat4("Row: ", (float*)&row);
    }

    ImGui::End();



    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Gray66()));
    commands.push_back(std::move(camera_command));


    // Draw the cubes.
    if (move_cubes)
      cubes_time_delta += game.time.frame_delta;
    float angle = cubes_time_delta * ToRadians(33.0f);
    for (uint32_t i = 0; i < ubos.size(); i++) {
      auto& ubo = ubos[i];

      float x = i % kMaxRow - 1;
      float y = i / kMaxRow;

      ubo.vert.model = Translate({x, y, -2});
      ubo.vert.model *= Rotate({0.5f, 0.33f, -0.2f}, angle * i * 0.4f);
      ubo.vert.model *= Scale(0.5f);

      ubo.frag.light.pos = light_pos;
      commands.push_back(
          CreateRenderCommand(&cube_mesh, &object_shader, &diffuse_map, &specular_map, ubo));
    }
    commands.push_back(
        CreateRenderCommand(&light_cube_mesh, &light_shader, nullptr, nullptr, light_ubo));
    commands.push_back(grid.render_command);


    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
