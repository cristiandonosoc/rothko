// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace rothko {

struct Model;

namespace gltf {

bool LoadModel(const std::string& path, Model* out);

}  // namespace gltf
}  // namespace rothko
