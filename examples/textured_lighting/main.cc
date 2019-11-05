// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>
#include <rothko/ui/imgui.h>
#include <rothko/widgets/widgets.h>
#include <rothko/scene/lines.h>

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
  if (!Init(game.renderer.get(), &imgui))
    return 1;

  Shader object_shader = simple_lighting::CreateLightShader(game.renderer.get());
  /* Shader light_shader = simple_lighting::CreateLightShader(game.renderer.get()); */
  if (!Valid(object_shader))
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

  // Lights.
  LightWidgetManager light_widgets;
  Shader point_light_shader = CreatePointLightShader(game.renderer.get());
  Mesh point_light_mesh = CreatePointLightMesh(game.renderer.get());
  Shader directional_light_shader = CreateDirectionalLightShader(game.renderer.get());
  Mesh directional_light_mesh = CreateDirectionalLightMesh(game.renderer.get());
  Init(&light_widgets, "light-widgets", &point_light_shader, &point_light_mesh,
       &directional_light_shader, &directional_light_mesh);

  // Line Manager.
  Shader line_shader = CreateLineShader(game.renderer.get());
  if (!Valid(line_shader))
    return 1;

  LineManager line_manager = {};
  if (!Init(game.renderer.get(), &line_shader, &line_manager, "line-manager"))
    return 1;

  auto scene_graph = std::make_unique<SceneGraph>();

  SceneNode* point_light_node = AddNode(scene_graph.get());

  SceneNode* dir_light_node = AddNode(scene_graph.get());
  dir_light_node->transform.position = {0, 1, 0};
  dir_light_node->transform.rotation = {kRadians45, kRadians180, -kRadians45};

  Vec3 cube_positions[] = {Vec3(0.0f, 0.0f, 0.0f),
                           Vec3(2.0f, 5.0f, -15.0f),
                           Vec3(-1.5f, -2.2f, -2.5f),
                           Vec3(-3.8f, -2.0f, -12.3f),
                           Vec3(2.4f, -0.4f, -3.5f),
                           Vec3(-1.7f, 3.0f, -7.5f),
                           Vec3(1.3f, -2.0f, -2.5f),
                           Vec3(1.5f, 2.0f, -2.5f),
                           Vec3(1.5f, 0.2f, -1.5f),
                           Vec3(-1.3f, 1.0f, -1.5f)};

  constexpr int kCubeCount = ARRAY_SIZE(cube_positions);
  SceneNode* cube_nodes[kCubeCount] = {};

  Update(scene_graph.get());

  // Create the UBOs.
  std::vector<simple_lighting::LightShaderUBO> point_light_ubos;
  point_light_ubos.reserve(kCubeCount);


  /* constexpr int kMaxRow = 4; */
  for (int i = 0; i < kCubeCount; i++) {
    SceneNode* node = AddNode(scene_graph.get());
    cube_nodes[i] = node;

    /* float x = i % kMaxRow - 1; */
    /* float y = i / kMaxRow; */

    float angle = ToRadians(20) * i;

    /* node->transform.position = {x, y, -2}; */
    node->transform.position = cube_positions[i];
    node->transform.rotation = {angle, -angle, 0};
    node->transform.scale *= 0.5f;

    simple_lighting::LightShaderUBO ubo = {};

    /* ubo.frag.light.ambient = {0.2f, 0.2f, 0.2f}; */
    ubo.frag.light.ambient = {};
    ubo.frag.light.diffuse = {0.5f, 0.5f, 0.5f};
    ubo.frag.light.specular = {1, 1, 1};

    ubo.frag.material.specular = ToVec3(Color::White());
    ubo.frag.material.shininess = 128;

    point_light_ubos.push_back(std::move(ubo));
  }


  std::vector<simple_lighting::LightShaderUBO> dir_light_ubos = point_light_ubos;

  SceneNode* ground_node = AddNode(scene_graph.get());
  ground_node->transform.position = {0, -0.5f, 0};
  ground_node->transform.scale = {10, 0.2f, 10};

  simple_lighting::LightShaderUBO ground_ubo ={};
  ground_ubo.frag.light.ambient = {};
  ground_ubo.frag.light.diffuse = {0.5f, 0.5f, 0.5f};
  ground_ubo.frag.light.specular = {1, 1, 1};
  ground_ubo.frag.material.specular = ToVec3(Color::White());
  ground_ubo.frag.material.shininess = 128;

  Update(scene_graph.get());

  bool running = true;

  float light_time_delta = 0;
  (void)light_time_delta;

  bool move_cubes = false;
  float cubes_time_delta = 0;

  bool move_point_light = true;

  while (running) {
    auto events = Update(&game);
    BeginFrame(&imgui, &game.window, &game.time, &game.input);
    Reset(&light_widgets);

    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(game.input, Key::kEscape)) {
      running = false;
      break;
    }

    if (KeyUpThisFrame(game.input, Key::kSpace))
      move_cubes = !move_cubes;

    if (KeyUpThisFrame(game.input, Key::kC))
      move_cubes = !move_cubes;

    /* if (KeyUpThisFrame(&game.input, Key::kA)) */
    /*   move_point_light = !move_point_light; */

    DefaultUpdateOrbitCamera(game.input, &camera);

    auto push_camera = GetPushCamera(camera);

    // Update the scene.

    if (move_point_light) {
      point_light_node->transform =
          TranslateWidget(TransformKind::kGlobal, push_camera, point_light_node->transform);
    } else {
      dir_light_node->transform =
          RotateWidget(TransformKind::kLocal, push_camera, dir_light_node->transform);
    }

    if (move_cubes) {
      cubes_time_delta += game.time.frame_delta;
      float angle = cubes_time_delta * ToRadians(7.0f);
      for (uint32_t i = 0; i < point_light_ubos.size(); i++) {
        SceneNode* cube_node = cube_nodes[i];
        cube_node->transform.rotation = {angle * i, -angle * i, 0};
      }
    }
    Update(scene_graph.get());
    Vec3 point_light_pos = PositionFromTransformMatrix(point_light_node->transform.world_matrix);
    /* Vec3 dir_light_dir = RotationFromTransformMatrix(dir_light_node->transform.world_matrix); */
    /* Vec3 dir_light_dir = ToVec3(dir_light_node->transform.world_matrix.row(0)); */
    /* Vec3 dir_light_dir = ToVec3(dir_light_node->transform.world_matrix.cols[0]); */

    /* Reset(&line_manager); */
    /* PushLine(&line_manager, {}, Normalize(dir_light_dir), Color::Blue()); */
    /* if (!Stage(game.renderer.get(), &line_manager)) */
    /*   return 1; */

    // Add the widgets.
    PushPointLight(&light_widgets, &point_light_node->transform, {1, 1, 1});
    PushDirectionalLight(&light_widgets, &dir_light_node->transform, {1, 1, 1});

    // Create the render commands.
    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Gray66()));
    commands.push_back(push_camera);

    // Point light cubes.
    for (uint32_t i = 0; i < point_light_ubos.size(); i++) {
      auto& ubo = point_light_ubos[i];
      ubo.vert.model = cube_nodes[i]->transform.world_matrix;
      ubo.vert.normal_matrix = Transpose(Inverse(ubo.vert.model));

      ubo.frag.light.pos = ToVec4(point_light_pos);
      commands.push_back(
          CreateRenderCommand(&cube_mesh, &object_shader, &diffuse_map, &specular_map, ubo));
    }

    /* // Directional light cubes. */
    /* for (uint32_t i = 0; i < dir_light_ubos.size(); i++) { */
    /*   auto& ubo = dir_light_ubos[i]; */
    /*   ubo.vert.model = Translate({0, 0, 4}); */
    /*   ubo.vert.model *= cube_nodes[i]->transform.world_matrix; */
    /*   ubo.vert.normal_matrix = Transpose(Inverse(ubo.vert.model)); */

    /*   // Directional lights expect 0 in the w coordinate. */
    /*   ubo.frag.light.pos = ToVec4(dir_light_dir, 0); */
    /*   commands.push_back( */
    /*       CreateRenderCommand(&cube_mesh, &object_shader, &diffuse_map, &specular_map, ubo)); */
    /* } */

    /* ground_ubo.vert.model = ground_node->transform.world_matrix; */
    /* ground_ubo.frag.light.pos = ToVec4(point_light_pos); */
    /* commands.push_back( */
    /*     CreateRenderCommand(&cube_mesh, &object_shader, nullptr, nullptr, ground_ubo)); */

    auto light_commands = GetRenderCommands(light_widgets);
    commands.insert(commands.end(), light_commands.begin(), light_commands.end());

    commands.push_back(GetRenderCommand(line_manager));
    commands.push_back(grid.render_command);

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
