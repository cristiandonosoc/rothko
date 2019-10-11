// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include "loader.h"

using namespace rothko;

namespace {

// Parsing Code ------------------------------------------------------------------------------------

}  // namespace

int main() {
  auto log_handle = InitLoggingSystem(true);
  auto platform = InitializePlatform();
  /* Game game = {}; */
  /* InitWindowConfig window_config = {}; */
  /* window_config.type = WindowType::kSDLOpenGL; */
  /* window_config.resizable = true; */
  /* /1* window_config.fullscreen = true; *1/ */
  /* window_config.screen_size = {1920, 1440}; */
  /* if (!InitGame(&game, &window_config, true)) */
  /*   return 1; */



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

  gltf::Scene out_scene;

  // Go over the scene.
  auto& scene = model.scenes[model.defaultScene];
  LOG(App, "Processing scene %s", scene.name.c_str());

  gltf::ProcessScene(model, scene, &out_scene);
}
