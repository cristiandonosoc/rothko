// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/logging/logging.h>
#include <rothko/logging/timer.h>
#include <rothko/math/math.h>
#include <rothko/platform/platform.h>
#include <rothko/scene/camera.h>
#include <rothko/ui/imgui.h>
#include <rothko/widgets/grid.h>
#include <rothko/widgets/widgets.h>
#include <rothko/window/sdl/sdl_definitions.h>
#include <rothko/window/window.h>
#include <third_party/imguizmo/ImGuizmo.h>

#include <sstream>
#include <thread>

#include "shader.h"

using namespace rothko;
using namespace rothko::imgui;

namespace {

bool Setup(Window*);

std::vector<UBO> ubos;

Mesh CreateMesh();
Camera CreateCamera();

PushCamera push_camera;

Texture LoadTexture(Renderer* renderer, const std::string& path) {
  Texture texture;
  if (!STBLoadTexture(path, TextureType::kRGBA, &texture))
    return {};

  if (!RendererStageTexture(renderer, &texture))
    return {};

  return texture;
}

uint32_t VecToColor(ImVec4 color) {
  // RGBA
  return ((uint8_t)(color.x * 255.0f) << 24) |
         ((uint8_t)(color.y * 255.0f) << 16) |
         ((uint8_t)(color.z * 255.0f) << 8) |
         ((uint8_t)(color.w * 255.0f));
}

Vertex3dUVColor CreateVertex(Vec3 pos, Vec2 uv, uint32_t color) {
  Vertex3dUVColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = color;

  return vertex;
}

void MatrixWidget(const Mat4& m) {
  Vec4 row = m.row(0);
  ImGui::InputFloat4("X", (float*)&row);

  row = m.row(1);
  ImGui::InputFloat4("Y", (float*)&row);

  row = m.row(2);
  ImGui::InputFloat4("Z", (float*)&row);

  row = m.row(3);
  ImGui::InputFloat4("W", (float*)&row);
}

}  // namespace

namespace {

bool Setup(Window* window) {
  // Window.
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitWindow(window, &window_config)) {
    ERROR(App, "Could not initialize window. Exiting.");
    return false;
  }

  return true;
}

struct Colors {
  // abgr
  static constexpr uint32_t kBlue=    0xff'ff'00'00;
  static constexpr uint32_t kGreen =  0xff'00'ff'00;
  static constexpr uint32_t kRed =    0xff'00'00'ff;
  static constexpr uint32_t kWhite =  0xff'ff'ff'ff;
};

Mesh CreateMesh() {
  Mesh mesh = {};
  mesh.name = "cube";

  mesh.vertex_type = VertexType::k3dUVColor;
  Vertex3dUVColor vertices[] = {
      // X
      CreateVertex({-0.5f, -0.5f, -0.5f}, {0, 0}, Colors::kBlue),
      CreateVertex({-0.5f, -0.5f,  0.5f}, {0, 1}, Colors::kGreen),
      CreateVertex({-0.5f,  0.5f,  0.5f}, {1, 1}, Colors::kWhite),
      CreateVertex({-0.5f,  0.5f, -0.5f}, {1, 0}, Colors::kRed),

      CreateVertex({ 0.5f, -0.5f, -0.5f}, {0, 0}, Colors::kBlue),
      CreateVertex({ 0.5f, -0.5f,  0.5f}, {0, 1}, Colors::kGreen),
      CreateVertex({ 0.5f,  0.5f,  0.5f}, {1, 1}, Colors::kWhite),
      CreateVertex({ 0.5f,  0.5f, -0.5f}, {1, 0}, Colors::kRed),

      // Y
      CreateVertex({-0.5f, -0.5f, -0.5f}, {0, 0}, Colors::kBlue),
      CreateVertex({ 0.5f, -0.5f, -0.5f}, {0, 1}, Colors::kGreen),
      CreateVertex({ 0.5f, -0.5f,  0.5f}, {1, 1}, Colors::kWhite),
      CreateVertex({-0.5f, -0.5f,  0.5f}, {1, 0}, Colors::kRed),

      CreateVertex({-0.5f,  0.5f, -0.5f}, {0, 0}, Colors::kBlue),
      CreateVertex({ 0.5f,  0.5f, -0.5f}, {0, 1}, Colors::kGreen),
      CreateVertex({ 0.5f,  0.5f,  0.5f}, {1, 1}, Colors::kWhite),
      CreateVertex({-0.5f,  0.5f,  0.5f}, {1, 0}, Colors::kRed),

      // Z
      CreateVertex({-0.5f, -0.5f, -0.5f}, {0, 0}, Colors::kBlue),
      CreateVertex({ 0.5f, -0.5f, -0.5f}, {0, 1}, Colors::kGreen),
      CreateVertex({ 0.5f,  0.5f, -0.5f}, {1, 1}, Colors::kWhite),
      CreateVertex({-0.5f,  0.5f, -0.5f}, {1, 0}, Colors::kRed),

      CreateVertex({-0.5f, -0.5f,  0.5f}, {0, 0}, Colors::kBlue),
      CreateVertex({ 0.5f, -0.5f,  0.5f}, {0, 1}, Colors::kGreen),
      CreateVertex({ 0.5f,  0.5f,  0.5f}, {1, 1}, Colors::kWhite),
      CreateVertex({-0.5f,  0.5f,  0.5f}, {1, 0}, Colors::kRed),
  };

  Mesh::IndexType indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,

    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,

    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
  };

  PushVertices(&mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&mesh, indices, ARRAY_SIZE(indices));

  ASSERT_MSG(mesh.vertex_count == ARRAY_SIZE(vertices), "Count: %u", mesh.vertex_count);
  /* ASSERT(mesh.vertices.size() == sizeof(vertices)); */

  ASSERT_MSG(mesh.indices.size() == ARRAY_SIZE(indices), "Count: %lu", mesh.indices.size());
  /* ASSERT(mesh.indices.size() == sizeof(indices)); */

  return mesh;
}

PerFrameVector<RenderCommand>
GetRenderCommands(Mesh* mesh, Shader* shader, Texture* tex0, Texture* tex1) {
  PerFrameVector<RenderCommand> commands;

  // Mesh command.
  RenderMesh render_mesh = {};
  render_mesh.mesh = mesh;
  render_mesh.shader = shader;
  render_mesh.primitive_type = PrimitiveType::kTriangles;
  ClearCullFaces(&render_mesh.flags);
  render_mesh.indices_count = mesh->indices.size();
  render_mesh.ubo_data[0] = (uint8_t*)&ubos[0];
  render_mesh.textures.push_back(tex1);
  render_mesh.textures.push_back(tex0);
  commands.push_back(render_mesh);

  return commands;
}

}  // namespace

int main() {
  auto log_handle = InitLoggingSystem(true);

  ERROR(App, "Test error: %s", "error");
  WARNING(OpenGL, "Test warning");

  Window window;
  if (!Setup(&window))
    return 1;

  auto renderer = InitRenderer();
  if (!renderer)
    return 1;

  // Initial config of the renderer.
  PushConfig initial_config = {};
  initial_config.viewport_pos = {};
  initial_config.viewport_size = window.screen_size;
  RendererExecuteCommands(renderer.get(), {initial_config});

  Input input = {};

  Mesh mesh = CreateMesh();
  if (!RendererStageMesh(renderer.get(), &mesh))
    return 1;

  auto shader = CreateShader(renderer.get());
  if (!shader)
    return 1;

  auto grid_shader = CreateGridShader(renderer.get(), "grid-shader");
  if (!grid_shader)
    return 1;

  Grid grid;
  if (!Init(&grid, renderer.get(), grid_shader.get()))
    return 1;

  Texture wall = LoadTexture(renderer.get(), "examples/cube/wall.jpg");
  if (!Loaded(wall))
    return 1;

  Texture face = LoadTexture(renderer.get(), "examples/cube/awesomeface.png");
  if (!Loaded(face))
    return 1;

  LineManager line_manager, axis_widget = {};
  if (!Init(&line_manager, renderer.get(), "line-manager") ||
      !Init(&axis_widget, renderer.get(), "axis-widget")) {
    return 1;
  }

  PushLine(&line_manager, {1, 1, 1}, {2, 2, 2}, Color::Blue());
  PushLine(&line_manager, {-3, 2, -3}, {0, 2, 2}, Color::Red());
  PushLine(&line_manager, {2, 2, -3}, {3, 2, -1}, Color::Green());
  PushCubeCenter(&line_manager, {1.5f, 1.5f, 1.5f}, {0.5f, 0.5f, 0.5f}, Color::White());
  PushCube(&line_manager, {-1, -1, -1}, {2, 4, 5}, Color::Black());

  PushRing(&line_manager, {}, Vec3::Up(), 1, Color::Blue());
  PushRing(&line_manager, {}, {1, 0, 0}, 1, Color::Green());
  PushRing(&line_manager, {}, {0, 0, 1}, 1, Color::Red());
  /* PushRing(&line_manager, {}, Vec3::Up(), 4, Color::Blue()); */


  constexpr int kRingCount = 10;
  constexpr float kRingAngle = kRadians90 / (float)kRingCount;

  for (int i = 0; i < 2 * kRingCount; i++) {
    float angle = i * kRingAngle;
    Vec3 normal = {Sin(angle), Cos(angle), 0};

    PushRing(&line_manager, {1 + 0.2f * i, 1, 1}, normal, 1 + 0.025f * i, Color::Black());
  }

  if (!Stage(&line_manager, renderer.get()))
    return 1;

  PushLine(&axis_widget, {}, {1, 0, 0}, Color::Red());
  PushLine(&axis_widget, {}, {0, 1, 0}, Color::Green());
  PushLine(&axis_widget, {}, {0, 0, 1}, Color::Blue());
  if (!Stage(&axis_widget, renderer.get()))
    return 1;

  float aspect_ratio = (float)window.screen_size.width / (float)window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  push_camera.view = GetView(camera);
  push_camera.projection = Perspective(ToRadians(60.0f), aspect_ratio, 0.1f, 100.0f);

  UBO ubo;
  ubo.model = Translate({0, 0, 0});
  ubos.push_back(ubo);
  ubo.model = Translate({10, 0, 0});
  ubos.push_back(ubo);
  ubo.model = Translate({0, 1, 0}) * Scale(0.5f);
  ubos.push_back(ubo);

  Time time = InitTime();

  ImguiContext imgui;
  if (!Init(renderer.get(), &imgui))
    return 1;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Sample game loop.
  bool running = true;
  while (running) {

    // Update --------------------------------------------------------------------------------------

    WindowEvent event = StartFrame(&window, &input);
    if (event == WindowEvent::kQuit) {
      running = false;
      break;
    }

    if (KeyUpThisFrame(input, Key::kEscape)) {
      running = false;
      break;
    }

    Update(&time);
    RendererStartFrame(renderer.get());
    BeginFrame(&imgui, &window, &time, &input);

    /* constexpr float kMouseSensibility = 0.007f; */
    /* static float kMaxPitch = ToRadians(89.0f); */
    /* if (!imgui.mouse_captured) { */
    /*   if (input.mouse.right) { */
    /*     if (!IsZero(input.mouse_offset)) { */
    /*       camera.angles.x -= input.mouse_offset.y * kMouseSensibility; */
    /*       if (camera.angles.x > kMaxPitch) { */
    /*         camera.angles.x = kMaxPitch; */
    /*       } else if (camera.angles.x < -kMaxPitch) { */
    /*         camera.angles.x = -kMaxPitch; */
    /*       } */

    /*       camera.angles.y += input.mouse_offset.x * kMouseSensibility; */
    /*       if (camera.angles.y > kRadians360) { */
    /*         camera.angles.y -= kRadians360; */
    /*       } else if (camera.angles.y < 0) { */
    /*         camera.angles.y += kRadians360; */
    /*       } */
    /*     } */
    /*   } */

    /*   // Zoom. */
    /*   if (input.mouse.wheel.y != 0) { */
    /*     // We actually want to advance a percentage of the distance. */
    /*     camera.distance -= input.mouse.wheel.y * camera.distance * camera.zoom_speed; */
    /*     if (camera.distance < 0.5f) */
    /*       camera.distance = 0.5f; */
    /*   } */
    /* } */

    /* Update(&camera); */
    /* push_camera.camera_pos = camera.pos_; */
    /* push_camera.view = GetView(camera); */
    /* push_camera.projection = GetProjection(camera); */

    // Create GUI ----------------------------------------------------------------------------------

    /* ImGui::ShowDemoWindow(); */
    CreateLogWindow();

    static ImGuizmo::OPERATION imguizmo_operation = ImGuizmo::TRANSLATE;
    static ImGuizmo::MODE imguizmo_mode = ImGuizmo::WORLD;

    {
      ImGui::Begin("Cube Example");

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);
      ImGui::ColorEdit3("clear color", (float*)&clear_color);

      ImGui::Separator();

      static int proj_option = (int)camera.projection_type;
      ImGui::RadioButton("Perspective", &proj_option, (int)ProjectionType::kProjection);
      ImGui::SameLine();
      ImGui::RadioButton("Ortho", &proj_option, (int)ProjectionType::kOrthographic);
      camera.projection_type = (ProjectionType)proj_option;

      Vec3 camera_pos = camera.pos_;
      ImGui::InputFloat3("Camera target", (float*)&camera.target);
      ImGui::SliderFloat("Camera distance", &camera.distance, 1.0f, 40.0f);
      ImGui::SliderFloat("Side/depth fix", &camera.size_per_depth_fix, 0.5f, 2.0f);
      ImGui::SliderFloat("Zoom speed (percent)", &camera.zoom_speed, 0.01f, 0.2f);

      float deg_angles[2] = {
        ToDegrees(camera.angles.x),
        ToDegrees(camera.angles.y),
      };
      ImGui::InputFloat2("Camera angles", deg_angles);

      camera.angles.x = ToRadians(deg_angles[0]);
      camera.angles.y = ToRadians(deg_angles[1]);

      ImGui::InputFloat3("Camera pos (fixed)", (float*)&camera_pos);

      ImGui::Separator();

      ImGui::Checkbox("Mouse captured", &imgui.mouse_captured);
      ImGui::InputInt2("Mouse", (int*)&input.mouse);
      ImGui::InputInt2("Mouse (prev)", (int*)&input.prev_mouse);
      ImGui::InputInt2("Mouse offset", (int*)&input.mouse_offset);

      ImGui::Separator();

      ImGui::Text("ROTHKO VIEW");
      MatrixWidget(push_camera.view);

      ImGui::Separator();

      ImGui::Text("Manipulation");

      ImGui::RadioButton("Translate", (int*)&imguizmo_operation, ImGuizmo::OPERATION::TRANSLATE);
      ImGui::SameLine();
      ImGui::RadioButton("Rotate", (int*)&imguizmo_operation, ImGuizmo::OPERATION::ROTATE);
      ImGui::SameLine();
      ImGui::RadioButton("Scale", (int*)&imguizmo_operation, ImGuizmo::OPERATION::SCALE);

      ImGui::RadioButton("World", (int*)&imguizmo_mode, ImGuizmo::MODE::WORLD);
      ImGui::SameLine();
      ImGui::RadioButton("Local", (int*)&imguizmo_mode, ImGuizmo::MODE::LOCAL);

      if (imguizmo_operation == ImGuizmo::OPERATION::SCALE)
        imguizmo_mode = ImGuizmo::MODE::LOCAL;

      ImGui::End();
    }

    DefaultUpdateOrbitCamera(input, &camera);
    push_camera = GetPushCamera(camera);


    // Generate render commands --------------------------------------------------------------------

    PerFrameVector<RenderCommand> commands;

    // Clear command.
    ClearFrame clear_frame;
    clear_frame = {};
    clear_frame.color = VecToColor(clear_color);
    commands.push_back(std::move(clear_frame));

    // Set the camera.
    commands.push_back(push_camera);

    /* commands.push_back(GetRenderCommand(line_manager)); */
    commands.push_back(grid.render_command);

    // Config the renderer for the axis.
    constexpr float kAxisWidgetSize = 0.10f;
    PushConfig axis_config = {};
    Vec2 axis_widget_size = Vec2(window.screen_size) * kAxisWidgetSize;

    axis_config.viewport_pos = window.screen_size - Int2(axis_widget_size * 1.05f);
    axis_config.viewport_size = ToInt2(axis_widget_size);
    commands.push_back(axis_config);

    OrbitCamera axis_camera = camera;
    axis_camera.target = {};
    axis_camera.distance = 1.25f;
    Update(&axis_camera);

    commands.push_back(GetPushCamera(axis_camera, ProjectionType::kOrthographic));
    commands.push_back(GetRenderCommand(axis_widget));

    /* /1* Mat4 identity = Mat4::Identity(); *1/ */
    /* ImGuizmo::Manipulate((float*)&push_camera.view, */
    /*                      (float*)&push_camera.projection, */
    /*                      imguizmo_operation, */
    /*                      imguizmo_mode, */
    /*                      (float*)&ubos[0]); */
    /*                      /1* (float*)&identity); *1/ */

    commands.push_back(PopConfig());
    commands.push_back(PopCamera());

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    commands.push_back(PopCamera());

    RendererExecuteCommands(renderer.get(), std::move(commands));

    RendererEndFrame(renderer.get(), &window);
  }
}


