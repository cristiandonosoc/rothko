// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include <rothko/platform/platform.h>

#include <rothko/game.h>

using namespace rothko;

namespace {

void ProcessNode(const tinygltf::Model& model, const tinygltf::Node& node) {
  if (node.mesh == -1) {
    WARNING(App, "Node %s has no mesh.", node.name.c_str());
    return;
  }

  const tinygltf::Mesh& mesh = model.meshes[node.mesh];
  for (const tinygltf::Primitive& primitive : mesh.primitives) {
    (void)primitive;
  }
}

void ProcessNodes(const tinygltf::Model& model, const tinygltf::Node& node) {
  ProcessNode(model, node);
  for (int node_index : node.children) {
    ProcessNodes(model, model.nodes[node_index]);
  }
}

}  // namespace

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

  // Go over the scene.
  auto& scene = model.scenes[model.defaultScene];
  for (int node_index : scene.nodes) {
    ProcessNodes(model, model.nodes[node_index]);
  }
}
