// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/graphics/default_shaders/default_shaders.h>
#include <rothko/models/cube.h>
#include <rothko/scene/camera.h>
#include <rothko/scene/grid.h>
#include <rothko/scene/scene_graph.h>
#include <rothko/ui/imgui.h>
#include <rothko/widgets/widgets.h>
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

void CreateGUI(SceneNode* root, WidgetOperation* op) {
  ImGui::Begin("Cube Example");

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate);

  ImGui::Separator();

  int op_int = (int)*op;
  ImGui::RadioButton("Translation", &op_int, (int)WidgetOperation::kTranslate);
  ImGui::RadioButton("Rotation", &op_int, (int)WidgetOperation::kRotate);
  ImGui::RadioButton("Scale", &op_int, (int)WidgetOperation::kScale);
  *op = (WidgetOperation)op_int;

  ImGui::Separator();

  ImGui::InputFloat3("Position", (float*)&root->transform.position);

  ImGui::Separator();

  for (int i = 0; i < 4; i++) {
    auto row = root->transform.world_matrix.row(i);
    ImGui::InputFloat4("", (float*)&row);
  }

  ImGui::End();
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
  SceneNode* grand_child = AddNode(scene_graph.get(), child);
  SceneNode* grand_child2 = AddNode(scene_graph.get(), child);
  SceneNode* child2 = AddNode(scene_graph.get(), root);

  child->transform.position = {3, 0, 0};
  child->transform.scale *= 0.5f;

  child2->transform.position = {0 ,0, 3};
  child2->transform.scale *= 0.5f;

  grand_child->transform.position = {2, 2, 0};
  grand_child->transform.scale *= 0.5f;
  grand_child2->transform.position = {0, -2, 2};
  grand_child2->transform.scale *= 0.5f;

  SceneNode* nodes[] = {
      root,
      child,
      grand_child,
      grand_child2,
      child2,
  };

  Update(scene_graph.get());

  ImguiContext imgui;
  if (!Init(game.renderer.get(), &imgui))
    return 1;


  /* Mat4 m = Mat4::Identity(); */
  bool op = false;

  WidgetOperation operation = WidgetOperation::kTranslate;

  SceneNode* current_node = nodes[0];
  bool running = true;
  while (running) {
    auto events = Update(&game);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    DefaultUpdateOrbitCamera(game.input, &camera);

    if (KeyUpThisFrame(&game.input, Key::kEscape)) {
      running = false;
      break;
    }

    BeginFrame(&imgui, &game.window, &game.time, &game.input);

    if (KeyUpThisFrame(&game.input, Key::kSpace))
      op = !op;

    uint32_t base = (uint32_t)Key::k1;
    for (int i = 0; i < ARRAY_SIZE(nodes); i++) {
      Key key = (Key)(base + i);
      if (KeyUpThisFrame(&game.input, key))
        current_node = nodes[i];
    }


    PushCamera push_camera = GetPushCamera(camera);
    ImGuizmo::SetRect(0, 0, game.window.screen_size.width, game.window.screen_size.height);

    SceneNode* parent = GetParent(scene_graph.get(), current_node);
    Transform* parent_transform = parent ? &parent->transform : nullptr;
    current_node->transform = TransformWidget(operation, TransformKind::kGlobal,
                                              push_camera,
                                              current_node->transform,
                                              parent_transform);

    Update(scene_graph.get());

    CreateGUI(root, &operation);

    // Create render commands.

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    commands.push_back(push_camera);

    for (SceneNode* node : nodes) {
      commands.push_back(GetCubeRenderCommand(&cube, &default_shader, node));
    }

    commands.push_back(grid.render_command);

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));

    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
