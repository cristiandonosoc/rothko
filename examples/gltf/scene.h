// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "rothko/graphics/mesh.h"

namespace rothko {

struct Model;
struct ModelInstance;

namespace gltf {

struct Scene {
  std::vector<std::unique_ptr<Model>> models;
  std::vector<ModelInstance> instances;
  bool valid = false;
};

#pragma pack(push, 1)

struct Header {
  char magic[4] = {};  // Should have 'R', 'T', 'H', 'K'.
  uint32_t version = 0;
  uint32_t next_mesh = 0;
};

// Meshes ------------------------------------------------------------------------------------------
//
// Format is:
//
// |--------------|
// | MeshesHeader |---- next_mesh ---|
// |--------------|                  |
//                                   |
//       ----------------------------|
//       |
//       |    |-------- next_mesh ------------------------|
//       |    |                                           |
//       v    |                                           v
//   |------------|                                 |------------|
//   | MeshHeader |---- data_offset ---|            | MeshHeader |
//   |------------|                    |            |------------|
//                                     |
//         ----------------------------|
//         |
//         v
//   |----------|
//   | Vertices |     // Vertices are always followed by indices, in one big chunk.
//   |----------|
//   | Indices  |
//   |----------|

struct MeshesHeader {
  uint32_t mesh_count = 0;
  uint32_t next_mesh = 0;     // Offset in bytes of the next header.
};

struct MeshHeader {
  uint32_t vertex_type = 0;
  uint32_t vertex_count = 0;
  uint32_t index_count = 0;

  uint32_t data = 0;            // Offset to where the data is.
  uint32_t next_mesh = 0;       // Offset in bytes to the next mesh.

  static constexpr uint32_t kNameLength = 64;
  char name[kNameLength] = {};

};

#pragma pack(pop)

bool SerializeScene(const Scene&, const std::string& path);
Scene ReadScene(const std::string& path);

}  // namespace gltf
}  // namespace rothko
