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

constexpr uint32_t kMaxMeshesPerNode = 4;

struct ModelNodeMesh {
  // Bounds on the mesh.
  Vec3 min = {};
  Vec3 max = {};
  const Mesh* mesh = nullptr;
  const Material* material = nullptr;
};
inline bool Valid(const ModelNodeMesh& m) { return !!m.mesh || !!m.material; }

struct ModelNode {
  ModelNodeMesh meshes[kMaxMeshesPerNode] = {};
  Transform transform;
};

struct Model {
  std::string name;

  std::vector<std::unique_ptr<Mesh>> meshes;
  std::map<int, std::unique_ptr<Texture>> textures;
  std::map<int, std::unique_ptr<Material>> materials;

  std::vector<ModelNode> nodes;
};

struct ModelTransformData {
  Mat4 transform;
  Mat4 inverse_transform;
};

struct ModelInstance {
  Model* model = nullptr;
  Transform transform = {};

  ModelTransformData* transform_data = nullptr;
};

bool ProcessModel(const tinygltf::Model&, const tinygltf::Scene&, Model* out_model);

}  // namespace gltf
}  // namespace rothko
