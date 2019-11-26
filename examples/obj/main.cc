// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/scene/camera.h>

#include <third_party/tiny_obj_loader/tiny_obj_loader.h>

using namespace rothko;

namespace {

std::unique_ptr<Game> InitGame() {
  auto game  = std::make_unique<Game>();

  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(game.get(), &window_config, true))
    return nullptr;

  return game;
}

bool LoadModel(const char* model_path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn, err;

  /* auto basepath = GetBasePath(model_path); */
  /* auto filename = GetBasename(model_path); */

  bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path);
  if (!warn.empty())
    WARNING(App, "%s", warn.c_str());
  if (!err.empty())
    ERROR(App, "%s", err.c_str());

  if (!result)
    return result;

  printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
  printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
  printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
  printf("# of materials = %d\n", (int)materials.size());
  printf("# of shapes    = %d\n", (int)shapes.size());

  return true;
}

}  // namespace

int main(int argc, const char* argv[]) {
  if (argc == 1) {
    fprintf(stderr, "Usage: obj <model path>\n");
    return 1;
  }

  auto game = InitGame();
  if (!game)
    return 1;

  if (!LoadModel(argv[1]))
    return 1;

  float aspect_ratio = (float)game->window.screen_size.x / (float)game->window.screen_size.y;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  bool running = true;
  while (running) {
    auto events = Update(game.get());
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(game->input, Key::kEscape)) {
      running = false;
      break;
    }

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    commands.push_back(GetPushCamera(camera));



    commands.push_back(PopCamera());
    RendererExecuteCommands(game->renderer.get(), std::move(commands));
    RendererEndFrame(game->renderer.get(), &game->window);
  }
}
