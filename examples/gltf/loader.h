// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/scene/transform.h>

#include <memory>

// Forward declarations.
namespace tinygltf {

class Model;
struct Scene;

}  // namespace gltf

namespace rothko {

struct Material;
struct Mesh;
struct Texture;

namespace gltf {

struct SceneNode {
  Mesh* mesh = nullptr;
  Material* material = nullptr;

  Transform transform;
};

struct Scene {
  std::vector<std::unique_ptr<Mesh>> meshes;
  std::vector<std::unique_ptr<Texture>> textures;

  std::vector<SceneNode> nodes;
};

void ProcessScene(const tinygltf::Model&, const tinygltf::Scene&, Scene* out_scene);

}  // namespace gltf
}  // namespace rothko
