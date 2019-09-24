// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include <rothko/platform/platform.h>

#include <rothko/game.h>

using namespace rothko;

int main() {
  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;


  std::string path = OpenFileDialog();
  if (path.empty()) {
    ERROR(App, "Could not get model path.");
    return 1;
  }

  std::string err, warn;

  tinygltf::TinyGLTF gltf_loader;
  tinygltf::Model model = {};
  if (!gltf_loader.LoadASCIIFromFile(&model, &err, &warn, path)) {
    ERROR(App, "Could not load model: %s", err.c_str());
    return 1;
  }

  if (!warn.empty()) {
    WARNING(App, "At loading model %s: %s", path.c_str(), warn.c_str());
  }

  LOG(App, "Loaded model!");

}
