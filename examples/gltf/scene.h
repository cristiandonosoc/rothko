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
  static const char* kTitle;

  char title[8] = {};
  uint32_t version = 0;

  uint32_t meshes = 0;    // Offset to a |MeshesHeader|.
  uint32_t textures = 0;  // Offset to a |TexturesHeader|.
};
static_assert(sizeof(Header) == 20);

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
  static const char* kTitle;

  char title[8] = {};
  uint32_t count = 0;
  uint32_t next_mesh = 0;     // Offset in bytes of the next header.
};
static_assert(sizeof(MeshesHeader) == 16);

struct MeshHeader {
  static const char* kTitle;

  char title[8] = {};
  uint32_t next_mesh = 0;       // Offset in bytes to the next mesh.
  uint32_t data = 0;            // Offset to where the data is.

  uint32_t vertex_type = 0;
  uint32_t vertex_count = 0;
  uint32_t index_count = 0;

  static constexpr uint32_t kNameLength = 64;
  char name[kNameLength] = {};
};
static_assert(sizeof(MeshHeader) == 92);

// Textures ----------------------------------------------------------------------------------------
//
// Format is:
//
// |----------------|
// | TexturesHeader |---- next_texture ---|
// |----------------|                     |
//                                        |
//       ---------------------------------|
//       |
//       |    |-------- next_texture ---------------------|
//       |    |                                           |
//       v    |                                           v
//   |---------------|                           |---------------|
//   | TextureHeader |---- data ----|            | TextureHeader |
//   |---------------|              |            |---------------|
//                                  |
//         -------------------------|
//         |
//         v
//   |----------|
//   | Contents |
//   |----------|

struct TexturesHeader {
  static const char* kTitle;

  char title[8] = {};
  uint32_t count = 0;
  uint32_t next_texture = 0;  // Offset in bytes to a |TextureHeader|.
};

struct TextureHeader {
  static const char* kTitle;

  char title[8] = {};
  uint32_t next_texture = 0;            // Offset in bytes towards next |TextureHeader|.
  uint32_t data = 0;                    // Offset in bytes.

  uint8_t type = UINT8_MAX;             // ::rothko::TextureType.

  uint32_t size_x = UINT32_MAX;
  uint32_t size_y = UINT32_MAX;

  uint8_t wrap_mode_u = UINT8_MAX;      // ::rothko::TextureWrapMode.
  uint8_t wrap_mode_v = UINT8_MAX;      // ::rothko::TextureWrapMode.

  uint8_t min_filter_mode = UINT8_MAX;  // ::rothko::TextureFilterMode.
  uint8_t mag_filter_mode = UINT8_MAX;  // ::rothko::TextureFilterMode.

  static constexpr uint32_t kNameLength = 64;
  char name[kNameLength] = {};

};

#pragma pack(pop)

bool SerializeScene(const Scene&, const std::string& path);
Scene ReadScene(const std::string& path);

}  // namespace gltf
}  // namespace rothko
