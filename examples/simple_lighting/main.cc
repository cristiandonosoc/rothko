// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/scene_graph.h>
#include <rothko/ui/imgui/imgui.h>
#include <rothko/utils/strings.h>
#include <rothko/widgets/grid.h>
#include <rothko/widgets/widgets.h>

#include "shaders.h"

using namespace rothko;

namespace {

template <typename UBO>
RenderMesh CreateRenderCommand(Mesh* mesh,
                               Shader* shader,
                               const UBO& ubo) {
  RenderMesh render_mesh = {};
  render_mesh.mesh = mesh;
  render_mesh.shader = shader;
  render_mesh.primitive_type = PrimitiveType::kTriangles;

  render_mesh.indices_count = mesh->indices.size();

  render_mesh.ubo_data[0] = (uint8_t*)&ubo.vert;
  render_mesh.ubo_data[1] = (uint8_t*)&ubo.frag;

  return render_mesh;
}

// Material table.

std::pair<std::string, simple_lighting::ObjectShaderUBO::Frag::Material> CreateMaterial(
    const std::string& name, Vec3 ambient, Vec3 diffuse, Vec3 specular, float shininess) {
  simple_lighting::ObjectShaderUBO::Frag::Material material = {};
  material.ambient = ambient;
  material.diffuse = diffuse;
  material.specular = specular;
  material.shininess = shininess;

  return {name, std::move(material)};
}

std::vector<std::pair<std::string, simple_lighting::ObjectShaderUBO::Frag::Material>> kMaterials = {

    CreateMaterial("emerald",
                   {0.0215f, 0.1745f, 0.0215f},
                   {0.07568f, 0.61424f, 0.07568f},
                   {0.633f, 0.727811f, 0.633f},
                   0.6f),
    CreateMaterial(
        "jade",
        {0.135f, 0.2225f, 0.1575f},
        {0.54f, 0.89f, 0.63f},
        {0.316228f, 0.316228f, 0.316228f}, 0.1f),
    CreateMaterial("obsidian",
                   {0.05375f, 0.05f, 0.06625f},
                   {0.18275f, 0.17f, 0.22525f},
                   {0.332741f, 0.328634f, 0.346435f},
                   0.3f),
    CreateMaterial("pearl",
                   {0.25f, 0.20725f, 0.20725f},
                   {1.00f, 0.829f, 0.829f},
                   {0.296648f, 0.296648f, 0.296648f},
                   0.088f),
    CreateMaterial("ruby",
                   {0.1745f, 0.01175f, 0.01175f},
                   {0.61424f, 0.04136f, 0.04136f},
                   {0.727811f, 0.626959f, 0.626959f},
                   0.6f),
    CreateMaterial("turquoise",
                   {0.10f, 0.18725f, 0.1745f},
                   {0.396f, 0.74151f, 0.69102f},
                   {0.297254f, 0.30829f, 0.306678f},
                   0.1f),
    CreateMaterial("brass",
                   {0.329412f, 0.223529f, 0.027451f},
                   {0.780392f, 0.568627f, 0.113725f},
                   {0.992157f, 0.941176f, 0.807843f},
                   0.21794872f),
    CreateMaterial("bronze",
                   {0.2125f, 0.1275f, 0.054f},
                   {0.714f, 0.4284f, 0.18144f},
                   {0.393548f, 0.271906f, 0.166721f},
                   0.2f),
};

struct AppContext {
  Vec4 clear_color = {0.45f, 0.55f, 0.60f, 1.00f};
  OrbitCamera camera = {};

  Vec3 light_pos = {};
  Vec3 light_ambient = {1, 1, 1};
  Vec3 light_diffuse = {0.3f, 0.3f, 0.3f};
  Vec3 light_specular = {0.5f, 0.5f, 0.5f};
};

simple_lighting::ObjectShaderUBO CreateUBO(const AppContext& context ,Vec3 pos, Vec3 scale) {
  simple_lighting::ObjectShaderUBO ubo = {};
  ubo.vert.model = Translate(pos);
  ubo.vert.model *= Scale(scale);

  ubo.frag.material.ambient = {1, 1, 1};
  ubo.frag.material.diffuse = {1, 1, 1};
  ubo.frag.material.specular = {1, 1, 1};
  ubo.frag.material.shininess = 128;

  /* ubo.frag.light.pos = {}; */
  /* ubo.frag.light.ambient = {0.3f, 0.3f, 0.3f}; */
  /* ubo.frag.light.diffuse = {0.5f, 0.5f, 0.5f}; */

  ubo.frag.light.pos = context.light_pos;
  ubo.frag.light.ambient = context.light_ambient;
  ubo.frag.light.diffuse = context.light_diffuse;
  ubo.frag.light.specular = context.light_specular;

  return ubo;
}

void CreateGUI(const imgui::ImguiContext& imgui, AppContext* context) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Options")) {
      ImGui::ColorEdit3("clear color", (float*)&context->clear_color);
      ImGui::EndMenu();
    }

    auto framerate_str = StringPrintf("Application average %.3f ms/frame (%.1f FPS)",
                                      1000.0f / ImGui::GetIO().Framerate,
                                      ImGui::GetIO().Framerate);
    float framerate_str_width = framerate_str.length() * imgui.font_size.x;

    ImGui::GetWindowWidth();

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - framerate_str_width);
    ImGui::Text("%s", framerate_str.c_str());

    ImGui::EndMainMenuBar();
  }

  ImGui::Begin("Simple Lighting");

  ImGui::Text("%s", "Camera");

  static int projection = 0;
  ImGui::RadioButton("Perspective", &projection, 0);
  ImGui::RadioButton("Ortho", &projection, 1);

  context->camera.projection_type =
      projection == 0 ? ProjectionType::kProjection : ProjectionType::kOrthographic;

  ImGui::InputFloat3("Pos", (float*)&context->camera.pos_);
  ImGui::InputFloat3("Target", (float*)&context->camera.target);
  ImGui::InputFloat2("Spherical Angles", (float*)&context->camera.angles);

  {
    ImGui::PushItemWidth(100.0f);
    float fov = ToDegrees(context->camera.fov);
    ImGui::InputFloat("FOV", &fov);
    ImGui::SameLine();
    ImGui::InputFloat("Aspect Ratio", &context->camera.aspect_ratio);
    ImGui::PopItemWidth();
  }

  ImGui::Separator();
  ImGui::Text("Light");

  ImGui::InputFloat3("Pos", (float*)&context->light_pos);
  ImGui::ColorEdit3("Ambient", (float*)&context->light_ambient);
  ImGui::ColorEdit3("Diffuse", (float*)&context->light_diffuse);
  ImGui::ColorEdit3("Specular", (float*)&context->light_specular);


  ImGui::End();
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

  PushConfig push_config = {};
  // TODO(Cristian): Actually find the height of the bar.
  push_config.viewport_pos = {};
  push_config.viewport_size = game.window.screen_size - Int2{0, 20};

  auto object_shader = simple_lighting::CreateObjectShader(game.renderer.get());
  if (!object_shader)
    return 1;

  AppContext app_context = {};

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  app_context.camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  auto grid_shader = CreateGridShader(game.renderer.get(), "grid-shader");
  if (!grid_shader)
    return 1;

  Grid grid;
  if (!Init(&grid, game.renderer.get(), grid_shader.get()))
    return 1;

  Mesh cube_mesh = CreateCubeMesh(VertexType::k3dNormal, "cube");
  if (!RendererStageMesh(game.renderer.get(), &cube_mesh))
    return 1;

  Mesh light_cube_mesh = CreateCubeMesh(VertexType::k3d, "light-cube");
  if (!RendererStageMesh(game.renderer.get(), &light_cube_mesh))
    return 1;

  auto line_shader = CreateLineShader(game.renderer.get(), "line-shader");
  if (!line_shader)
    return 1;


  /* LightWidgetManager light_widgets; */
  /* auto point_light_shader = CreatePointLightShader(game.renderer.get()); */
  /* auto directional_light_shader = CreateDirectionalLightShader(game.renderer.get()); */
  /* Init(&light_widgets, */
  /*      game.renderer.get(), */
  /*      "light-widgets", */
  /*      point_light_shader.get(), */
  /*      directional_light_shader.get(), */
  /*      line_shader.get()); */


  LightWidgetManager light_widgets;
  if (!Init(&light_widgets, game.renderer.get(), "light-widgets"))
    return 1;


  auto scene_graph = std::make_unique<SceneGraph>();

  SceneNode* base_light = AddNode(scene_graph.get());
  SceneNode* light_node = AddNode(scene_graph.get(), base_light);
  light_node->transform.position = {3, 3, 3};
  light_node->transform.scale *= 0.2f;

  // Create the UBOs.
  std::vector<simple_lighting::ObjectShaderUBO> ubos;
  ubos.reserve(kMaterials.size());

  ubos.push_back(CreateUBO(app_context, {0, 0, 0}, {10, 0.5f, 10}));
  for (float z = 0; z < 5; z++) {
    for (float x = 0; x < 5; x++) {
      float xx = -4.0f + x * 2;
      float zz = -4.0f + z * 2;
      ubos.push_back(CreateUBO(app_context, {xx, 2, zz}, {0.5f, 3, 0.5f}));
    }
  }

  Update(scene_graph.get());

  imgui::ImguiContext imgui;
  if (!Init(game.renderer.get(), &imgui))
    return 1;

  bool running = true;
  /* float time_delta = 0; */
  while (running) {
    // Frame start.
    WindowEvent event = StartFrame(&game);
    if (event == WindowEvent::kQuit) {
      running = false;
      break;
    }

    BeginFrame(&imgui, &game.window, &game.time, &game.input);
    Reset(&light_widgets);
    DefaultUpdateOrbitCamera(game.input, &app_context.camera);

    if (KeyUpThisFrame(game.input, Key::kEscape)) {
      running = false;
      break;
    }

    // Update Scene.

    auto push_camera = GetPushCamera(app_context.camera);
    light_node->transform =
        TranslateWidget(TransformKind::kGlobal, push_camera, light_node->transform, nullptr);
    Update(scene_graph.get());

    app_context.light_pos = light_node->transform.position;

    // Add the widgets.
    /* PushPointLight(&light_widgets, &light_node->transform, app_context.light_diffuse); */
    PushDirectionalLight(&light_widgets, &light_node->transform, app_context.light_diffuse);

    // Create Commands.

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Gray66()));
    commands.push_back(GetPushCamera(app_context.camera));

    // Draw the cubes.
    for (auto& ubo : ubos) {
      // Update the light.
      ubo.frag.light.pos = app_context.light_pos;
      ubo.frag.light.ambient = app_context.light_ambient;
      ubo.frag.light.diffuse = app_context.light_diffuse;
      ubo.frag.light.specular = app_context.light_specular;

      commands.push_back(CreateRenderCommand(&cube_mesh, object_shader.get(), ubo));
    }

    auto light_commands = GetRenderCommands(light_widgets);
    commands.insert(commands.end(), light_commands.begin(), light_commands.end());

    commands.push_back(grid.render_command);

    commands.push_back(PopCamera());
    CreateGUI(imgui, &app_context);

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
