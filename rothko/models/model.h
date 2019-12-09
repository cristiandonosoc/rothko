// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "rothko/math/math.h"
#include "rothko/scene/transform.h"

namespace rothko {

struct Material;
struct Mesh;
struct Texture;

struct ModelPrimitive {
  const Mesh* mesh = nullptr;
  const Material* material = nullptr;

  Bounds bounds = {};   // In local space.
};
inline bool Valid(const ModelPrimitive& p) { return !!p.mesh && !!p.material; }

constexpr uint32_t kMaxPrimitivesPerModelNode = 4;

struct ModelNode {
  ModelPrimitive primitives[kMaxPrimitivesPerModelNode] = {};
  Transform transform;
};

struct Model {
  std::string name;

  std::vector<std::unique_ptr<Material>> materials;
  std::vector<std::unique_ptr<Mesh>> meshes;
  std::vector<std::unique_ptr<Texture>> textures;

  std::vector<ModelNode> nodes;
};

// Instances.

struct ModelTransform {
  Mat4 transform;
  Mat4 inverse_transform;
};

struct ModelInstance {
  Model* model = {};
  Transform transform = {};
};

}  // namespace rothko
