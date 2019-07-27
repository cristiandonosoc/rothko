// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/math/math.h>
#include <rothko/platform/timing.h>
#include <rothko/scene/camera.h>
#include <rothko/logging/logging.h>
#include <rothko/window/sdl/sdl_definitions.h>
#include <rothko/window/window.h>
#include <rothko/ui/imgui.h>
#include <rothko/logging/timer.h>

#include <sstream>
#include <thread>

#include "gui.h"

using namespace rothko;
using namespace rothko::imgui;

namespace {

bool Setup(Window*, Renderer*);

struct UBO {
  Mat4 proj;
  Mat4 view;
  Mat4 model;
};

std::vector<UBO> ubos;

struct CubeShader {
  Shader shader;
};

Mesh CreateMesh();
CubeShader CreateShader();
Camera CreateCamera();

PerFrameVector<RenderCommand> GetRenderCommands(Mesh* mesh, CubeShader* shader);

}  // namespace

int main() {
  /* LogContainer::Init(); */
  /* LogContainer::Init(); */

  auto handle1 = InitLoggingSystem();


  Window window;
  Renderer renderer;
  if (!Setup(&window, &renderer))
    return 1;

  Input input = {};

  Mesh mesh = CreateMesh();
  if (!RendererStageMesh(&renderer, &mesh))
    return 1;

  CubeShader cube_shader = CreateShader();
  if (!RendererStageShader(&renderer, &cube_shader.shader))
    return 1;

  float aspect_ratio = (float)window.screen_size.width / (float)window.screen_size.height;

  UBO ubo;
  ubo.proj= Perspective(ToRadians(60.0f), aspect_ratio, 0.1f, 100.0f);
  ubo.view = LookAt({5, 5, 5}, {}, {0, 1, 0});
  ubo.model = Translate({10, 0, 0});
  ubos.push_back(ubo);
  ubo.model = Translate({0, 0, 0});
  ubos.push_back(ubo);

  Time time = InitTime();

  ImguiContext imgui;
  if (!InitImgui(&renderer, &imgui))
    return 1;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  Timings timings = {};

  // Sample game loop.
  int frame_count = 0;
  bool running = true;
  while (running) {
    Timer timer = Timer::CreateAndStart();

    auto events = NewFrame(&window, &input);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(&input, Key::kEscape)) {
      running = false;
      break;
    }

    timings.event_count = input.event_count;

    timings.frame_delta = time.frame_delta;

    Update(&time);
    StartFrame(&renderer);
    StartFrame(&imgui, &window, &time, &input);

    timings.start_frame = timer.End();

    timer = Timer::CreateAndStart();

    PerFrameVector<RenderCommand> commands;

    float angle = time.seconds * ToRadians(50.0f);
    ubos[1].model = Rotate({1.0f, 0.3f, 0.5f}, angle);
    auto cube_commands = GetRenderCommands(&mesh, &cube_shader);
    commands.insert(commands.end(), cube_commands.begin(), cube_commands.end());

    CreateDebugGui(timings);

    timings.create_my_commands = timer.End();

    timer = Timer::CreateAndStart();

    /* ImGui::ShowDemoWindow(); */


    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
    // window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.

      ImGui::Text(
          "This is some useful text.");  // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window",
                      &show_demo_window);  // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

      if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true
                                    // when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);
      ImGui::End();
    }

    auto imgui_commands = EndFrame(&imgui);
   commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    timings.create_imgui_commands = timer.End();

    timer = Timer::CreateAndStart();

    RendererExecuteCommands(commands, &renderer);

    timings.execute_commands = timer.End();

    timer = Timer::CreateAndStart();

    EndFrame(&renderer);

    timings.end_frame = timer.End();

    frame_count++;
    /* std::this_thread::sleep_for(std::chrono::milliseconds(16)); */
  }
}

namespace {

bool Setup(Window* window, Renderer* renderer) {
  // Window.
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitWindow(window, &window_config)) {
    LOG(ERROR, "Could not initialize window. Exiting.");
    return false;
  }

  // Renderer.
  InitRendererConfig renderer_config = {};
  renderer_config.type = RendererType::kOpenGL;
  renderer_config.window = window;
  if (!InitRenderer(renderer, &renderer_config)) {
    LOG(ERROR, "Could not initialize the renderer. Exiting.");
    return false;
  }

  return true;
}

struct Colors {
  // abgr
  /* static constexpr uint32_t kBlack=   0x00'00'00'ff; */
  /* static constexpr uint32_t kBlue=    0x00'00'ff'ff; */
  /* static constexpr uint32_t kGreen =  0x00'ff'00'ff; */
  /* static constexpr uint32_t kRed =    0xff'00'00'ff; */
  /* static constexpr uint32_t kWhite =  0xff'ff'ff'ff; */
  /* static constexpr uint32_t kTeal =   0xff'f9'f0'ea; */
  /* static constexpr uint32_t kGray =   0xff'99'99'99; */

  /* static constexpr uint32_t kBlack=   0xff'00'00'00; */
  static constexpr uint32_t kBlue=    0xff'ff'00'00;
  static constexpr uint32_t kGreen =  0xff'00'ff'00;
  static constexpr uint32_t kRed =    0xff'00'00'ff;
  static constexpr uint32_t kWhite =  0xff'ff'ff'ff;
};

VertexColor CreateVertex(Vec3 pos, Vec2 uv, uint32_t color) {
  VertexColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = color;

  return vertex;
}

Mesh CreateMesh() {
  Mesh mesh = {};
  mesh.name = "cube";
  mesh.vertex_type = VertexType::kColor;

  VertexColor vertices[] = {
    // X

    CreateVertex({-1, -1, -1},  {-1, -1}, Colors::kBlue),
    CreateVertex({-1, -1,  1},  {-1,  1}, Colors::kGreen),
    CreateVertex({-1,  1,  1},  { 1,  1}, Colors::kWhite),
    CreateVertex({-1,  1, -1},  { 1, -1}, Colors::kRed),

    CreateVertex({-1, -1, -1}, {-1, -1}, Colors::kBlue),
    CreateVertex({-1, -1,  1}, {-1,  1}, Colors::kGreen),
    CreateVertex({-1,  1,  1}, { 1,  1}, Colors::kWhite),
    CreateVertex({-1,  1, -1}, { 1, -1}, Colors::kRed),

    // Y
    CreateVertex({-1, -1, -1}, {-1, -1}, Colors::kBlue),
    CreateVertex({ 1, -1, -1}, {-1, -1}, Colors::kGreen),
    CreateVertex({ 1, -1,  1}, {-1,  1}, Colors::kWhite),
    CreateVertex({-1, -1,  1}, {-1,  1}, Colors::kRed),

    CreateVertex({-1,  1, -1}, { 1, -1}, Colors::kBlue),
    CreateVertex({ 1,  1, -1}, { 1, -1}, Colors::kGreen),
    CreateVertex({ 1,  1,  1}, { 1,  1}, Colors::kWhite),
    CreateVertex({-1,  1,  1}, { 1,  1}, Colors::kRed),

    // Z
    CreateVertex({-1, -1, -1}, {-1, -1}, Colors::kBlue),
    CreateVertex({ 1, -1, -1}, {-1, -1}, Colors::kGreen),
    CreateVertex({ 1,  1, -1}, { 1, -1}, Colors::kWhite),
    CreateVertex({-1,  1, -1}, { 1, -1}, Colors::kRed),
    CreateVertex({-1, -1,  1}, {-1,  1}, Colors::kBlue),
    CreateVertex({ 1, -1,  1}, {-1,  1}, Colors::kGreen),
    CreateVertex({ 1,  1,  1}, { 1,  1}, Colors::kWhite),
    CreateVertex({-1,  1,  1}, { 1,  1}, Colors::kRed),
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

  ASSERT_MSG(mesh.vertices_count == ARRAY_SIZE(vertices), "Count: %u", mesh.vertices_count);
  ASSERT(mesh.vertices.size() == sizeof(vertices));

  ASSERT_MSG(mesh.indices_count == ARRAY_SIZE(indices), "Count: %u", mesh.indices_count);
  ASSERT(mesh.indices.size() == sizeof(indices));

  return mesh;
}

CubeShader CreateShader() {
  CubeShader shader;
  shader.shader.name = "cube";
  shader.shader.vert_ubo = {"Uniforms", sizeof(UBO)};

  ASSERT(LoadShaderSources("examples/cube/shader.vert",
                           "examples/cube/shader.frag",
                           &shader.shader));
  return shader;
}

PerFrameVector<RenderCommand>
GetRenderCommands(Mesh* mesh, CubeShader* cube_shader) {
  (void)mesh;
  (void)cube_shader;
  PerFrameVector<RenderCommand> commands;

  // Clear command.
  ClearFrame clear_frame;
  clear_frame = {};
  clear_frame.color = 0x002266ff;
  commands.push_back(std::move(clear_frame));

  // Mesh command.
  RenderMesh render_mesh;
  render_mesh.mesh = mesh;
  render_mesh.shader = &cube_shader->shader;
  render_mesh.cull_faces = false;
  render_mesh.indices_size = mesh->indices_count;
  render_mesh.vert_ubo_data = (uint8_t*)&ubos[0];
  commands.push_back(render_mesh);

  render_mesh.mesh = mesh;
  render_mesh.shader = &cube_shader->shader;
  render_mesh.cull_faces = false;
  render_mesh.indices_size = mesh->indices_count;
  render_mesh.vert_ubo_data = (uint8_t*)&ubos[1];
  commands.push_back(render_mesh);

  return commands;
}

}  // namespace
