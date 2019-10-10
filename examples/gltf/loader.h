// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once


// Forward declarations.
namespace tinygltf {

class Model;
struct Scene;

}  // namespace gltf

namespace rothko {
namespace gltf {

void ProcessScene(const tinygltf::Model&, const tinygltf::Scene&);

}  // namespace gltf
}  // namespace rothko
