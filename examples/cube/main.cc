// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/scene/camera.h>
#include <rothko/utils/logging.h>
#include <rothko/window/sdl/sdl_definitions.h>
#include <rothko/window/window.h>
#include <rothko/math/vec.h>

#include <thread>

using namespace rothko;

namespace {

bool Setup(Window*, Renderer*);

Mesh CreateMesh();
Shader CreateShader();
Camera CreateCamera();

PerFrameVector<RenderCommand> GetRenderCommands(Camera* camera, Mesh* mesh, Shader* shader);

}  // namespace


int main() {
  Window window;
  Renderer renderer;
  if (!Setup(&window, &renderer))
    return 1;

  Input input = {};

  Mesh mesh = CreateMesh();
  if (!RendererStageMesh(&renderer, &mesh))
    return 1;

  Shader shader = CreateShader();
  if (!RendererStageShader(&renderer, &shader))
    return 1;

  Camera camera;
  camera.projection = Mat4::Identity();
  camera.view = LookAt({0, 0, 5}, {}, {0, 1, 0});

  // Sample game loop.
  bool running = true;
  while (running) {
    auto events = NewFrame(&window, &input);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    StartFrame(&renderer);

    auto commands = GetRenderCommands(&camera, &mesh, &shader);
    RendererExecuteCommands(commands, &renderer);

    EndFrame(&renderer);

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
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

  static constexpr uint32_t kBlack=   0xff'00'00'00;
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
    {{-1, -1, 0}, Colors::kBlue},
    {{ 1, -1, 0}, Colors::kGreen},
    {{ 1,  1, 0}, Colors::kWhite},
    {{-1,  1, 0}, Colors::kRed},
  };

  Mesh::IndexType indices[] = {
    0, 1, 2,
    2, 3, 0,
  };

  PushVertices(&mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&mesh, indices, ARRAY_SIZE(indices));

  ASSERT(mesh.vertices_count == 4);
  ASSERT(mesh.vertices.size() == sizeof(vertices));

  ASSERT_MSG(mesh.indices_count == 6, "Count: %u", mesh.indices_count);
  ASSERT(mesh.indices.size() == sizeof(indices));

  return mesh;
}

Shader CreateShader() {
  Shader shader = {};
  shader.name = "cube";

  shader.vert_ubo = {"Camera", 128};

  ASSERT(LoadShaderSources("examples/cube/shader.vert",
                           "examples/cube/shader.frag",
                           &shader));
  return shader;
}

PerFrameVector<RenderCommand> GetRenderCommands(Camera* camera, Mesh* mesh, Shader* shader) {
  (void)camera;
  (void)mesh;
  (void)shader;

  PerFrameVector<RenderCommand> commands;

  // Clear command.
  RenderCommand command;
  command.type = RenderCommandType::kClear;
  auto& clear_action = command.ClearAction();
  clear_action = {};
  clear_action.color = Colors::kBlue;
  commands.push_back(std::move(command));

  // Mesh command.
  command = {};
  command.type = RenderCommandType::kMesh;
  command.shader = shader;

  MeshRenderAction mesh_action;
  mesh_action.mesh = mesh;
  mesh_action.indices_size = mesh->indices_count;
  mesh_action.vert_ubo_data = (uint8_t*)camera;

  command.MeshActions().push_back(std::move(mesh_action));

  commands.push_back(std::move(command));

  return commands;
}

}  // namespace
