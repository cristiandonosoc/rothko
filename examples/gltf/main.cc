// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include <rothko/graphics/default_shaders/default_shaders.h>

#include "loader.h"

using namespace rothko;

namespace {

// Parsing Code ------------------------------------------------------------------------------------

}  // namespace

int main(int argc, const char* argv[]) {

  std::string path;
  if (argc == 2)
    path = argv[1];


  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;

  if (path.empty()) {
    path = OpenFileDialog();
    if (path.empty()) {
      ERROR(App, "Could not get model path.");
      return 1;
    }
  }

  LOG(App, "Path: %s", path.c_str());

  std::string err, warn;

  tinygltf::TinyGLTF gltf_loader;
  tinygltf::Model model = {};
  if (!gltf_loader.LoadASCIIFromFile(&model, &err, &warn, path)) {
    ERROR(App, "Could not load model: %s", err.c_str());
    return 1;
  }

  if (!warn.empty()) {
    WARNING(App, "At loading model %s: %s", path.c_str(), warn.c_str());
  }

  LOG(App, "Loaded model!");

  gltf::Scene scene;

  // Go over the scene.
  auto& gltf_scene = model.scenes[model.defaultScene];
  LOG(App, "Processing scene %s", gltf_scene.name.c_str());

  gltf::ProcessScene(model, gltf_scene, &scene);

  // -----------------------------------------------------------------------------------------------


  if (!RendererStageMesh(game.renderer.get(), scene.meshes[0].get()))
    return 1;

  if (!RendererStageTexture(game.renderer.get(), scene.textures[0].get()))
    return 1;


  Grid grid;
  if (!Init(game.renderer.get(), &grid, "main-grid"))
    return 1;

  Shader default_shader = CreateDefaultShader(VertexType::k3dNormalTangentUV);
  if (!Valid(default_shader) || !RendererStageShader(game.renderer.get(), &default_shader))
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  Mat4 model_mat = Mat4::Identity();

  bool running = true;
  while (running) {
    auto events = Update(&game);
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

    /* StartFrame(&imgui, &game.window, &game.time, &game.input); */

    /* ImGui::Begin("Cube Example"); */

    /* ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", */
    /*             1000.0f / ImGui::GetIO().Framerate, */
    /*             ImGui::GetIO().Framerate); */

    /* ImGui::End(); */



    DefaultUpdateOrbitCamera(game.input, &camera);

    /* float angle = game.time.seconds * ToRadians(20.0f); */
    /* root->transform.rotation.x = -angle; */
    /* root->transform.rotation.y = angle; */

    /* float child_angle = game.time.seconds * ToRadians(33.0f); */
    /* child->transform.position = {2 * Cos(child_angle), 0, 2 * Sin(child_angle)}; */
    /* child2->transform.position = {0, 2 * Cos(2 * child_angle), 2 * Sin(2 * child_angle)}; */
    /* grand_child->transform.position = {0, 1, 0}; */

    /* Update(scene_graph.get()); */

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    commands.push_back(GetCommand(camera));

    auto* mesh = scene.meshes[0].get();

    RenderMesh render_mesh = {};
    render_mesh.mesh = mesh;
    render_mesh.shader = &default_shader;
    render_mesh.primitive_type = PrimitiveType::kTriangles;
    render_mesh.indices_count = mesh->indices.size();
    render_mesh.vert_ubo_data = (uint8_t*)&model_mat;

    commands.push_back(std::move(render_mesh));

    /* commands.push_back(GetCubeRenderCommand(&cube, &default_shader, root)); */
    /* commands.push_back(GetCubeRenderCommand(&cube, &default_shader, child)); */
    /* commands.push_back(GetCubeRenderCommand(&cube, &default_shader, child2)); */
    /* commands.push_back(GetCubeRenderCommand(&cube, &default_shader, grand_child)); */
    commands.push_back(grid.render_command);

    /* auto imgui_commands = EndFrame(&imgui); */
    /* commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end()); */

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
