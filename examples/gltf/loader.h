// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/scene/transform.h>

#include <memory>
#include <map>

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

struct ModelNode {
  Mesh* mesh = nullptr;
  Material* material = nullptr;

  Transform transform;

  // The bounds that considers this node and any children nodes.
  Vec3 min;
  Vec3 max;
};

struct Model {
  std::map<int, std::unique_ptr<Mesh>> meshes;
  std::map<int, std::unique_ptr<Texture>> textures;
  std::map<int, std::unique_ptr<Material>> materials;

  std::vector<ModelNode> nodes;
};

void ProcessModel(const tinygltf::Model&, const tinygltf::Scene&, Model* out_model);

}  // namespace gltf
}  // namespace rothko
