// Copyright 2020, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/logging/logging.h>
#include <rothko/models/model.h>
#include <rothko/utils/file.h>

#include "scene.h"

namespace rothko {
namespace gltf {

namespace {

// Verification ------------------------------------------------------------------------------------

// Returns the wrong title if bool is false.
template <typename HEADER_TYPE>
std::pair<bool, std::string> VerifyTitle(const HEADER_TYPE& header) {
  char title[ARRAY_SIZE(header.title) + 1] = {};
  memcpy(title, header.title, ARRAY_SIZE(header.title));
  if (strcmp(title, HEADER_TYPE::kTitle))
    return {false, title};
  return {true, {}};
}

bool Verify(const Header& header) {
  if (auto [valid, invalid_title] = VerifyTitle(header); !valid) {
    LOG(Model, "Invalid header title: %s.", invalid_title.c_str());
    return false;
  }

  return true;
}

bool Verify(const MeshesHeader& meshes, uint32_t offset) {
  if (auto [valid, invalid_title] = VerifyTitle(meshes); !valid) {
    LOG(Model, "Offset %u: Invalid header title: %s.", offset, invalid_title.c_str());
    return false;
  }

  return true;
}

bool Verify(const MeshHeader& mesh, uint32_t offset) {
  if (auto [valid, invalid_title] = VerifyTitle(mesh); !valid) {
    LOG(Model, "Offset %u: Invalid header title: %s.", offset, invalid_title.c_str());
    return false;
  }

  if (mesh.vertex_type >= (uint32_t)VertexType::kLast) {
    LOG(Model, "Offset %u: Wrong vertex_type: %u.",
               offset + (uint32_t)offsetof(MeshHeader, vertex_type), mesh.vertex_type);
    return false;
  }

  return true;
}

bool Verify(const TexturesHeader& textures, uint32_t offset) {
  if (auto [valid, invalid_title] = VerifyTitle(textures); !valid) {
    LOG(Model, "Offset %u: Invalid header title: %s.", offset, invalid_title.c_str());
    return false;
  }

  return true;
}

bool Verify(const TextureHeader& texture, uint32_t offset) {
  if (auto [valid, invalid_title] = VerifyTitle(texture); !valid) {
    LOG(Model, "Offset %u: Invalid header title: %s.", offset, invalid_title.c_str());
    return false;
  }

  if (texture.type >= (uint8_t)TextureType::kLast) {
    LOG(Model, "Offset %u: Wrong texture type: %u.",
               offset + (uint32_t)offsetof(TextureHeader, type), texture.type);
    return false;
  }

  if (texture.wrap_mode_u >= (uint8_t)TextureWrapMode::kLast ||
      texture.wrap_mode_v >= (uint8_t)TextureWrapMode::kLast) {
    LOG(Model, "Offset %u: Wrong UV wrap modes: (%u, %u).",
               offset + (uint32_t)offsetof(TextureHeader, wrap_mode_u),
               texture.wrap_mode_u,
               texture.wrap_mode_v);
    return false;
  }

  if (texture.min_filter_mode >= (uint8_t)TextureFilterMode::kLast ||
      texture.mag_filter_mode >= (uint8_t)TextureFilterMode::kLast) {
    LOG(Model, "Offset %u: Wrong filter modes: (%u, %u).",
               offset + (uint32_t)offsetof(TextureHeader, min_filter_mode),
               texture.min_filter_mode,
               texture.mag_filter_mode);
    return false;
  }

  return true;
}

// Extraction --------------------------------------------------------------------------------------

std::vector<Mesh> ExtractMeshes(const Header* header, const uint8_t* data) {
  printf("MeshesHeader Offset (bytes): %u\n", header->meshes);

  auto meshes = (const MeshesHeader*)(data + header->meshes);
  if (!Verify(*meshes, (uint8_t*)meshes - data))
    return {};

  printf("Meshes count: %u\n", meshes->count);

  auto mesh = (const MeshHeader*)(data + meshes->next_mesh);
  for (uint32_t i = 0; i < meshes->count; i++) {
    if (!Verify(*mesh, (uint8_t*)mesh - data))
      return {};

    VertexType vertex_type = ToVertexType(mesh->vertex_type);
    printf("Mesh %u: Vertex Type %s: %u\n", i, ToString(vertex_type), mesh->vertex_type);
    printf("Mesh %u: Vertex Count: %u\n", i, mesh->vertex_count);
    printf("Mesh %u: Index Count: %u\n", i, mesh->index_count);

    char name[MeshHeader::kNameLength + 1] = {};
    memcpy(name, mesh->name, MeshHeader::kNameLength);
    printf("Mesh %u: Name: %s\n", i, name);

    mesh = (const MeshHeader*)(data + mesh->next_mesh);
  }

  return {};
}

std::vector<Texture> ExtractTextures(const Header* header, const uint8_t* data) {
  printf("TexturesHeader Offset (bytes): %u\n", header->textures);

  auto textures = (const TexturesHeader*)(data + header->textures);
  if (!Verify(*textures, (uint8_t*)textures - data))
    return {};

  printf("Textures count: %u\n", textures->count);

  return {};
}

}  // namespace

// ReadScene ---------------------------------------------------------------------------------------

Scene ReadScene(const std::string& path) {
  std::vector<uint8_t> file_data;
  if (!ReadWholeFile(path, &file_data))
    return {};

  printf("READING -----------------------------------------------------------------------------\n");

  // Verify header.
  const uint8_t* data = file_data.data();
  auto header = (const Header*)data;
  if (!Verify(*header)) {
    printf("Invalid file header!\n");
    return {};
  }

  printf("Header version: %u\n", header->version);
  ExtractMeshes(header, data);
  ExtractTextures(header, data);

  return {};
}

}  // namespace gltf
}  // namespace rothko
