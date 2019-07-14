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

#include <sstream>
#include <thread>

using namespace rothko;
using namespace rothko::imgui;

/* BEGIN_IGNORE_WARNINGS() */

/* #include <third_party/include/glm/gtc/matrix_transform.hpp> */
/* #include <third_party/include/glm/gtc/type_ptr.hpp> */

/* #define GLM_ENABLE_EXPERIMENTAL */
/* #include <glm/gtx/string_cast.hpp> */

/* END_IGNORE_WARNINGS() */

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
  Logger logger = Logger::CreateLogger();
  /* auto my_ortho = Ortho(0, 600, 0, 480); */
  /* auto ortho = glm::ortho(0.0f, 600.0f, 0.0f, 480.0f); */

  /* LOG(DEBUG, "MINE: %s", ToString(my_ortho).c_str()); */
  /* LOG(DEBUG, " GLM: %s", ToString(*(Mat4*)&ortho).c_str()); */
  /* return 0; */

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

  float aspect_ratio = (float)window.width / (float)window.height;

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

  // Sample game loop.
  int frame_count = 0;
  bool running = true;
  while (running) {
    auto events = NewFrame(&window, &input);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }


    Update(&time);
    StartFrame(&renderer);
    StartFrame(&imgui, &window, &time, &input);


    /* ImGui::ShowDemoWindow(nullptr); */

    ImGui::Begin("Test");
    ImGui::Text("Hola");
    ImGui::End();

    float angle = time.seconds * ToRadians(50.0f);
    ubos[1].model = Rotate({1.0f, 0.3f, 0.5f}, angle);


    auto commands = GetRenderCommands(&mesh, &cube_shader);

    auto imgui_commands = EndFrame(&imgui);
    /* printf("Imgui commands size: %zu", imgui_commands.size()); */
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());
    /* for (RenderCommand& command : imgui_commands) { */
    /*   command.GetRenderMesh().shader = &cube_shader.shader; */
    /*   commands.push_back(std::move(command)); */
    /* } */

    RendererExecuteCommands(commands, &renderer);

    EndFrame(&renderer);

    frame_count++;
    if (frame_count == 2)
      break;

    /* std::this_thread::sleep_for(std::chrono::milliseconds(16)); */
  }
}

namespace {

bool Setup(Window* window, Renderer* renderer) {
  // Window.
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
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

Mesh CreateMesh() {
  Mesh mesh = {};
  mesh.name = "cube";
  mesh.vertex_type = VertexType::kColor;

  VertexColor vertices[] = {
    // X
    {{-1, -1, -1}, Colors::kBlue},
    {{-1, -1,  1}, Colors::kGreen},
    {{-1,  1,  1}, Colors::kWhite},
    {{-1,  1, -1}, Colors::kRed},
    {{-1, -1, -1}, Colors::kBlue},
    {{-1, -1,  1}, Colors::kGreen},
    {{-1,  1,  1}, Colors::kWhite},
    {{-1,  1, -1}, Colors::kRed},

    // Y
    {{-1, -1, -1}, Colors::kBlue},
    {{ 1, -1, -1}, Colors::kGreen},
    {{ 1, -1,  1}, Colors::kWhite},
    {{-1, -1,  1}, Colors::kRed},
    {{-1,  1, -1}, Colors::kBlue},
    {{ 1,  1, -1}, Colors::kGreen},
    {{ 1,  1,  1}, Colors::kWhite},
    {{-1,  1,  1}, Colors::kRed},

    // Z
    {{-1, -1, -1}, Colors::kBlue},
    {{ 1, -1, -1}, Colors::kGreen},
    {{ 1,  1, -1}, Colors::kWhite},
    {{-1,  1, -1}, Colors::kRed},
    {{-1, -1,  1}, Colors::kBlue},
    {{ 1, -1,  1}, Colors::kGreen},
    {{ 1,  1,  1}, Colors::kWhite},
    {{-1,  1,  1}, Colors::kRed},
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
