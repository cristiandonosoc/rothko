// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>
#include <rothko/ui/imgui.h>
#include <rothko/widgets/widgets.h>

#include "shaders.h"

using namespace rothko;
using namespace imgui;
using LightProperties = textured_lighting::FullLightUBO::Frag::LightProperties;
using PointLightProperties = textured_lighting::FullLightUBO::Frag::PointLightProperties;

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

  // Shaders ---------------------------------------------------------------------------------------

  Shader spot_light_shader = textured_lighting::CreateSpotLightShader(game.renderer.get());
  if (!Valid(spot_light_shader))
    return 1;

  Shader full_light_shader = textured_lighting::CreateFullLightShader(game.renderer.get());
  if (!Valid(full_light_shader))
    return 1;

  // Line Manager.
  Shader line_shader = CreateLineShader(game.renderer.get());
  if (!Valid(line_shader))
    return 1;

  // Camera ----------------------------------------------------------------------------------------

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  // Meshes ----------------------------------------------------------------------------------------

  Mesh cube_mesh = CreateCubeMesh(VertexType::k3dNormalUV, "cube");
  if (!RendererStageMesh(game.renderer.get(), &cube_mesh))
    return 1;

  Mesh light_cube_mesh = CreateCubeMesh(VertexType::k3d, "light-cube");
  if (!RendererStageMesh(game.renderer.get(), &light_cube_mesh))
    return 1;

  // Textures --------------------------------------------------------------------------------------

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

  // Widgets ---------------------------------------------------------------------------------------

  Grid grid;
  if (!Init(game.renderer.get(), &grid, "main-grid"))
    return 1;

  LineManager line_manager = {};
  if (!Init(&line_manager, game.renderer.get(), &line_shader, "line-manager"))
    return 1;

  // LightsWidgetManager ---------------------------------------------------------------------------

  LightWidgetManager light_widgets;
  Shader point_light_shader = CreatePointLightShader(game.renderer.get());
  Mesh point_light_mesh = CreatePointLightMesh(game.renderer.get());
  Shader directional_light_shader = CreateDirectionalLightShader(game.renderer.get());
  Mesh directional_light_mesh = CreateDirectionalLightMesh(game.renderer.get());
  Init(&light_widgets, game.renderer.get(), "light-widgets", &point_light_shader, &point_light_mesh,
       &directional_light_shader, &directional_light_mesh, &line_shader);

  // Scene Graph / UBOS ----------------------------------------------------------------------------

  auto scene_graph = std::make_unique<SceneGraph>();

  // Light nodes.

  SceneNode* dir_light_node = AddNode(scene_graph.get());
  dir_light_node->transform.position = {0, 5, 0};
  dir_light_node->transform.rotation = {kRadians45, kRadians180, -kRadians45};

  struct ExamplePointLight {
    SceneNode* node;
    LightProperties properties;
    PointLightProperties point_light_properties;
  };

  ExamplePointLight point_lights[textured_lighting::kPointLightCount] = {};
  for (int i = 0; i < textured_lighting::kPointLightCount; i++) {
    point_lights[i].node = AddNode(scene_graph.get());

    point_lights[i].properties.ambient = {};
    point_lights[i].properties.diffuse = {1, 1, 1};
    point_lights[i].properties.specular = {1, 1, 1};
  }

  point_lights[0].node->transform.position = {2, 0, 0};
  point_lights[1].node->transform.position = {-3, -3, 2};
  point_lights[2].node->transform.position = {0, 2, 0};
  point_lights[3].node->transform.position = {3, 1, -4};

  struct ExampleDirectionalLight {
    SceneNode* node;
    LightProperties properties;
  } dir_light = {};

  dir_light.node = AddNode(scene_graph.get());
  dir_light.properties.ambient = {};
  dir_light.properties.diffuse = {0.5f, 0.5f, 0.5f};
  dir_light.properties.specular = {1, 1, 1};

  // Cubes nodes.

  struct Cube {
    SceneNode* node;
    textured_lighting::FullLightUBO ubo;
  };

  std::vector<Cube> cubes;

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

  constexpr int kCubeCount = std::size(cube_positions);
  for (int i = 0; i < kCubeCount; i++) {
    auto& cube = cubes.emplace_back();

    cube.node = AddNode(scene_graph.get());

    float angle = ToRadians(20) * i;
    cube.node->transform.position = cube_positions[i];
    cube.node->transform.rotation = {angle, -angle, 0};
    cube.node->transform.scale *= 0.5f;

    cube.ubo.frag.material.specular = ToVec3(Color::White());
    cube.ubo.frag.material.shininess = 128;
  }

  Cube ground_cube = {};
  ground_cube.node = AddNode(scene_graph.get());
  ground_cube.node->transform.position = {0, -0.5f, 0};
  ground_cube.node->transform.scale = {10, 0.2f, 10};
  ground_cube.ubo.frag.material.specular = ToVec3(Color::White());
  ground_cube.ubo.frag.material.shininess = 128;

  Update(scene_graph.get());

  // Create light widgets --------------------------------------------------------------------------

  /* SpotLight spot_light = {}; */
  /* spot_light.position = {1, 1, 1}; */
  /* spot_light.direction = {0, -1, 0.1f}; */
  /* spot_light.transform = &spot_light_node->transform; */
  /* spot_light.angle = ToRadians(30); */
  /* spot_light.color = Color::Blue(); */
  /* PushSpotLight(&light_widgets, spot_light); */
  /* Stage(&light_widgets, game.renderer.get()); */

  // Begin Game loop -------------------------------------------------------------------------------

  bool running = true;

  float light_time_delta = 0;
  (void)light_time_delta;

  bool move_cubes = false;
  float cubes_time_delta = 0;

  bool move_point_light = true;

  ExamplePointLight* editing_point_light = point_lights;

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

    // Controls

    constexpr int kBase = (int)Key::k1;
    for (uint32_t i = 0; i < std::size(point_lights); i++) {
      if (KeyUpThisFrame(game.input, (Key)(kBase + i))) {
        editing_point_light = point_lights + i;
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

    if (KeyUpThisFrame(game.input, Key::kQ))
      move_point_light = !move_point_light;

    DefaultUpdateOrbitCamera(game.input, &camera);

    auto push_camera = GetPushCamera(camera);

    // Update the scene.

    TranslateWidget(TransformKind::kGlobal, push_camera, &editing_point_light->node->transform);

    if (move_cubes) {
      cubes_time_delta += game.time.frame_delta;
      float angle = cubes_time_delta * ToRadians(7.0f);
      for (uint32_t i = 0; i < cubes.size(); i++) {
        SceneNode* cube_node = cubes[i].node;
        cube_node->transform.rotation = {angle * i, -angle * i, 0};
      }
    }
    Update(scene_graph.get());

    // Add the widgets -----------------------------------------------------------------------------

    for (uint32_t i = 0; i < std::size(point_lights); i++) {
      auto& light = point_lights[i];
      PushPointLight(&light_widgets, &light.node->transform, light.properties.diffuse);
    }

    /* PushSpotLight(&light_widgets, spot_light); */
    Stage(&light_widgets, game.renderer.get());

    // Create the UI -------------------------------------------------------------------------------

    {

      /* ImGui::ShowDemoWindow(); */

      ImGui::Begin("Textured Lighting");

      if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputFloat3("Pos", (float*)&editing_point_light->node->transform.position);
        ImGui::ColorEdit3("Ambient", (float*)&editing_point_light->properties.ambient);
        ImGui::ColorEdit3("Diffuse", (float*)&editing_point_light->properties.diffuse);
        ImGui::ColorEdit3("Specular", (float*)&editing_point_light->properties.specular);
      }

      ImGui::End();
    }

    // Create the render commands ------------------------------------------------------------------

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Gray66()));
    commands.push_back(push_camera);

    // Draw cubes.
    for (uint32_t i = 0; i < std::size(cubes); i++) {
      auto& cube = cubes[i];
      cube.ubo.vert.model = cube.node->transform.world_matrix;
      cube.ubo.vert.normal_matrix = Transpose(Inverse(cube.ubo.vert.model));

      // Point Light positions.
      for (uint32_t point_i = 0; point_i < std::size(point_lights); point_i++) {
        auto& light = point_lights[point_i];
        auto& light_ubo = cube.ubo.frag.point_lights[point_i];

        light_ubo.position = ToVec4(GetWorldPosition(light.node->transform));
        light_ubo.properties = light.properties;
        light_ubo.point_light_properties = light.point_light_properties;
      }

      commands.push_back(CreateRenderCommand(
          &cube_mesh, &full_light_shader, &diffuse_map, &specular_map, cube.ubo));
    }

    /* // Point light cubes. */
    /* for (uint32_t i = 0; i < point_light_ubos.size(); i++) { */
    /*   auto& ubo = point_light_ubos[i]; */
    /*   ubo.vert.model = cube_nodes[i]->transform.world_matrix; */
    /*   ubo.vert.normal_matrix = Transpose(Inverse(ubo.vert.model)); */

    /*   ubo.frag.light.pos = ToVec4(point_light_pos); */
    /*   commands.push_back( */
    /*       CreateRenderCommand(&cube_mesh, &object_shader, &diffuse_map, &specular_map, ubo)); */
    /* } */

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
    /* ground_ubo.frag.light.pos = GetWorldPosition(*spot_light.transform); */
    /* ground_ubo.frag.light.direction = GetWorldDirection(*spot_light.transform); */
    /* ground_ubo.frag.light.cutoff_cos = Cos(spot_light.angle); */
    /* commands.push_back( */
    /*     CreateRenderCommand(&cube_mesh, &spot_light_shader, nullptr, nullptr, ground_ubo)); */

    auto light_commands = GetRenderCommands(light_widgets);
    commands.insert(commands.end(), light_commands.begin(), light_commands.end());

    commands.push_back(GetRenderCommand(line_manager));
    commands.push_back(grid.render_command);

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    // End frame -----------------------------------------------------------------------------------

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
