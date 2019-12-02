// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/graphics/default_shaders/default_shaders.h>
#include <rothko/math/math.h>
#include <rothko/scene/camera.h>
#include <rothko/widgets/grid.h>
#include <rothko/widgets/widgets.h>
#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include "loader.h"
#include "shaders.h"

using namespace rothko;

namespace {

std::pair<Vec3, Vec3> GetBounds(const Vec3& m1, const Vec3& m2) {
  Vec3 min = {};
  Vec3 max = {};

  min.x = Min(m1.x, m2.x);
  min.y = Min(m1.y, m2.y);
  min.z = Min(m1.z, m2.z);

  max.x = Max(m1.x, m2.x);
  max.y = Max(m1.y, m2.y);
  max.z = Max(m1.z, m2.z);

  return {min, max};
}

}  // namespace

int main(int argc, const char* argv[]) {
  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;

  std::string path;
  if (argc == 2)
    path = argv[1];


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
  tinygltf::Model gltf_model = {};
  if (!gltf_loader.LoadASCIIFromFile(&gltf_model, &err, &warn, path)) {
    ERROR(App, "Could not load model: %s", err.c_str());
    return 1;
  }

  if (!warn.empty()) {
    WARNING(App, "At loading model %s: %s", path.c_str(), warn.c_str());
  }

  LOG(App, "Loaded model!");

  gltf::Model model;

  // Go over the model.
  auto& gltf_scene = gltf_model.scenes[gltf_model.defaultScene];
  LOG(App, "Processing scene %s", gltf_scene.name.c_str());

  gltf::ProcessModel(gltf_model, gltf_scene, &model);

  LOG(App, "Meshes: %zu", model.meshes.size());
  LOG(App, "Textures: %zu", model.textures.size());

  LOG(App, "Mesh 0 vertex count %u", model.meshes[0]->vertex_count);
  LOG(App, "Mesh 0 index count %zu", model.meshes[0]->indices.size());

  // -----------------------------------------------------------------------------------------------

  for (auto& [id, mesh] : model.meshes) {
    if (!RendererStageMesh(game.renderer.get(), mesh.get()))
      return 1;
  }

  for (auto& [id, texture] : model.textures) {
    if (!RendererStageTexture(game.renderer.get(), texture.get()))
      return 1;
  }

  Grid grid;
  if (!Init(&grid, game.renderer.get()))
    return 1;

  auto model_shader = gltf::CreateModelShader(game.renderer.get());
  if (!model_shader)
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  LineManager lines = {};
  if (!Init(&lines, game.renderer.get(), "lines"))
    return 1;

  Mat4 model_transform = Mat4::Identity();

  bool running = true;
  bool rotate = false;
  float time = 0;
  float angle = 0;

  while (running) {
    WindowEvent event = StartFrame(&game);
    if (event == WindowEvent::kQuit) {
      running = false;
      break;
    }

    if (KeyUpThisFrame(game.input, Key::kEscape)) {
      running = false;
      break;
    }

    Reset(&lines);
    DefaultUpdateOrbitCamera(game.input, &camera);

    if (KeyUpThisFrame(game.input, Key::kSpace))
      rotate = !rotate;

    if (rotate) {
      time += game.time.frame_delta;
      angle = ToRadians(20.0f) * time;
    }
    model_transform = Rotate({0, 1, 0}, angle);

    // Create Commands.

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    commands.push_back(GetPushCamera(camera));

    for (auto& node : model.nodes) {
      if (!node.mesh)
        continue;

      RenderMesh render_mesh = {};
      render_mesh.mesh = node.mesh;
      render_mesh.shader = model_shader.get();
      render_mesh.primitive_type = PrimitiveType::kTriangles;
      render_mesh.indices_count = node.mesh->indices.size();

      render_mesh.ubo_data[0] = (uint8_t*)&model_transform;
      render_mesh.ubo_data[1] = (uint8_t*)&node.transform.world_matrix;
      render_mesh.ubo_data[2] = (uint8_t*)&node.material->base_color;
      render_mesh.textures = {node.material->base_texture};

      commands.push_back(std::move(render_mesh));

      /* Vec3 m1 = ToVec3(model_transform * node.transform.world_matrix * node.min); */
      /* Vec3 m2 = ToVec3(model_transform * node.transform.world_matrix * node.max); */

      auto t = model_transform * node.transform.world_matrix;
      Vec3 m1 = ToVec3(t * node.min);
      Vec3 m2 = ToVec3(t * node.max);

      auto [min, max] = GetBounds(m1, m2);
      PushCube(&lines, min, max, Color::Black());

      PushCubeCenter(&lines, m1, {0.1f, 0.1f, 0.1f}, Color::White());
      PushCubeCenter(&lines, m2, {0.1f, 0.1f, 0.1f}, Color::White());

      PushCubeCenter(&lines, min, Vec3{0.1f, 0.1f, 0.1f} * 2, Color::Red());
      PushCubeCenter(&lines, max, Vec3{0.1f, 0.1f, 0.1f} * 2, Color::Red());
    }

    commands.push_back(grid.render_command);
    if (!Stage(&lines, game.renderer.get()))
        return 2;

    auto cmd = GetRenderCommand(lines);
    commands.push_back(GetRenderCommand(lines));

    /* auto imgui_commands = EndFrame(&imgui); */
    /* commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end()); */

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));
    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
