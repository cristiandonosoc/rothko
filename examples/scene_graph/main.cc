// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/graphics/default_shaders/default_shaders.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>
#include <rothko/ui/imgui.h>

#include <third_party/imguizmo/ImGuizmo.h>

using namespace rothko;
using namespace rothko::imgui;

namespace {

RenderMesh GetCubeRenderCommand(Mesh* mesh, Shader* shader, SceneNode* node) {
  RenderMesh render_mesh = {};

  render_mesh.mesh = mesh;
  render_mesh.shader = shader;
  render_mesh.primitive_type = PrimitiveType::kTriangles;
  render_mesh.cull_faces = false;
  render_mesh.indices_count = mesh->indices.size();
  render_mesh.vert_ubo_data = (uint8_t*)&node->transform.world_matrix;
  /* render_mesh.textures.push_back(tex1); */
  /* render_mesh.textures.push_back(tex0); */

  return render_mesh;
}

}  // namespace

int main() {
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  Game game;
  if (!InitGame(&game, &window_config, false))
    return 1;

  Grid grid;
  if (!Init(game.renderer.get(), &grid, "main-grid"))
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  Shader default_shader = CreateDefaultShader(VertexType::k3dUVColor);
  if (!Valid(default_shader) || !RendererStageShader(game.renderer.get(), &default_shader))
    return 1;

  Mesh cube = CreateCubeMesh(VertexType::k3dUVColor, "cube");
  if (!RendererStageMesh(game.renderer.get(), &cube))
    return 1;

  auto scene_graph = std::make_unique<SceneGraph>();
  SceneNode* root = AddNode(scene_graph.get());

  SceneNode* child = AddNode(scene_graph.get(), root);
  child->transform.scale = {0.3f, 0.3f, 0.3f};

  SceneNode* child2 = AddNode(scene_graph.get(), root);
  child2->transform.scale = {0.1f, 0.1f, 0.1f};

  SceneNode* grand_child = AddNode(scene_graph.get(), child);
  grand_child->transform.scale = {0.7f, 0.3f, 1.3f};

  ImguiContext imgui;
  if (!InitImgui(game.renderer.get(), &imgui))
    return 1;


  Mat4 m = Mat4::Identity();
  bool op = false;

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

    StartFrame(&imgui, &game.window, &game.time, &game.input);
    ImGuizmo::BeginFrame();

    ImGui::Begin("Cube Example");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ImGui::Separator();

    for (int i = 0; i < 4; i++) {
      auto row = m.row(i);
      ImGui::InputFloat4("", (float*)&row);
    }

    ImGui::Separator();

    // Extract translation.
    Vec3 translation = {m.get(0, 3), m.get(1, 3), m.get(2, 3)};
    ImGui::InputFloat3("Translation: ", (float*)&translation);

    // Extract Scale.
    Vec3 scale = {};
    scale.x = Length(ToVec3(m.row(0)));
    scale.y = Length(ToVec3(m.row(1)));
    scale.z = Length(ToVec3(m.row(2)));

    ImGui::InputFloat3("Scale", (float*)&scale);

    // Extract rotation.
    float m00 = m.get(0, 0);
    float m10 = m.get(1, 0);

    float c2 = Sqrt(m00 * m00 + m10 * m10);

    Vec3 rotation = {};
    rotation.x = -Atan2(m.get(1, 2), m.get(2, 2));
    rotation.y = Atan2(-m.get(0, 2), c2);
    ImGui::InputFloat3("Rotation", (float*)&rotation);
    /* ImGui::InputFloat("Rx", &rx); */


    ImGui::End();

    if (KeyUpThisFrame(&game.input, Key::kSpace))
      op = !op;

    auto operation = ImGuizmo::OPERATION::ROTATE;
    if (op)
      operation = ImGuizmo::OPERATION::SCALE;

    PushCamera push_camera = GetPushCamera(camera);
    ImGuizmo::SetRect(0, 0, game.window.screen_size.width, game.window.screen_size.height);
    ImGuizmo::Manipulate((float*)&push_camera.view,
                         (float*)&push_camera.projection,
                         operation,
                         ImGuizmo::MODE::WORLD,
                         (float*)&m);
                         /* (float*)&identity); */




    DefaultUpdateOrbitCamera(game.input, &camera);

    float angle = game.time.seconds * ToRadians(20.0f);
    root->transform.rotation.x = -angle;
    root->transform.rotation.y = angle;

    float child_angle = game.time.seconds * ToRadians(33.0f);
    child->transform.position = {2 * Cos(child_angle), 0, 2 * Sin(child_angle)};
    child2->transform.position = {0, 2 * Cos(2 * child_angle), 2 * Sin(2 * child_angle)};
    grand_child->transform.position = {0, 1, 0};

    Update(scene_graph.get());

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    commands.push_back(push_camera);

    Transform transform;
    transform.position = {-3, 0, 0};
    transform.rotation = rotation;
    transform.scale = scale;
    Update(&transform);

    SceneNode node = {};
    /* m.get(0, 3) = 3; */
    node.transform.world_matrix = m;
    node.transform.world_matrix.get(0, 3) = 3;
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, &node));

    SceneNode node2 = node;
    node2.transform = transform;
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, &node2));

    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, root));
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, child));
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, child2));
    commands.push_back(GetCubeRenderCommand(&cube, &default_shader, grand_child));
    commands.push_back(grid.render_command);

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
