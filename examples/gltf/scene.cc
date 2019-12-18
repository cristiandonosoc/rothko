// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "scene.h"

#include <rothko/graphics/graphics.h>
#include <rothko/logging/logging.h>
#include <rothko/models/model.h>
#include <rothko/utils/file.h>

namespace rothko {
namespace gltf {

// Header Titles -----------------------------------------------------------------------------------

const char* Header::kTitle = "**RTHK**";
const char* MeshesHeader::kTitle = "*MESHES*";
const char* MeshHeader::kTitle = "**MESH**";

// Write -------------------------------------------------------------------------------------------

namespace {

constexpr uint32_t kVersion = 1;

Header CalculateHeader(const Scene& scene) {
  (void)scene;
  Header header = {};
  memcpy(header.title, Header::kTitle, std::size(header.title));
  header.version = kVersion;

  return header;
}

std::tuple<MeshesHeader, std::vector<MeshHeader>, std::vector<const Mesh*>>
CalculateMeshes(const Scene& scene) {
  MeshesHeader meshes_header = {};
  memcpy(meshes_header.title, MeshesHeader::kTitle, std::size(meshes_header.title));

  // Get the meshes.
  std::vector<const Mesh*> meshes;
  for (const auto& model : scene.models) {
    for (const auto& mesh : model->meshes) {
      meshes.push_back(mesh.get());
    }
  }

  meshes_header.count = meshes.size();
  printf("MESH COUNT: %u.\n", meshes_header.count);

  // Go over the meshes.
  std::vector<MeshHeader> mesh_headers;
  meshes.reserve(meshes.size());
  for (auto* mesh : meshes) {
    MeshHeader mesh_header = {};
    memcpy(mesh_header.title, MeshHeader::kTitle, std::size(mesh_header.title));

    mesh_header.vertex_type = (uint32_t)mesh->vertex_type;
    mesh_header.vertex_count = mesh->vertex_count;
    mesh_header.index_count = mesh->indices.size();

    printf("Vertex Type %s: %u.\n", ToString(mesh->vertex_type), (uint32_t)mesh->vertex_type);
    printf("Vertex count: %u.\n", mesh_header.vertex_count);
    printf("Index count: %u.\n", mesh_header.index_count);

    for (uint32_t i = 0; i < MeshHeader::kNameLength - 2; i++) {
      if (i >= mesh->name.size())
        break;
      mesh_header.name[i] = mesh->name[i];
    }

    mesh_headers.push_back(std::move(mesh_header));
  }

  return {meshes_header, std::move(mesh_headers), std::move(meshes)};
}

uint32_t DetermineMeshOffsets(uint32_t base_offset,
                              MeshesHeader* meshes_header,
                              std::vector<MeshHeader>* mesh_headers) {
  meshes_header->next_mesh = base_offset + sizeof(MeshesHeader);
  base_offset = meshes_header->next_mesh;

  for (auto& mesh_header : *mesh_headers) {
    mesh_header.next_mesh = base_offset + sizeof(MeshHeader);
    base_offset = mesh_header.next_mesh;
  }

  return base_offset;
}

std::tuple<TexturesHeader, std::vector<TextureHeader>, std::vector<const Texture*>>
CalculateTextures(const Scene& scene) {
  TexturesHeader textures_header = {};
  memcpy(textures_header.title, TexturesHeader::kTitle, std::size(textures_header.title));

  // Collect the textures.
  std::vector<const Texture*> textures;
  for (const auto& model : scene.models) {
    for (const auto& texture : model->textures) {
      textures.push_back(texture.get());
    }
  }

  textures_header.count = textures.size();
  printf("TEXTURES COUNT: %u.\n", textures_header.count);

  // Go over the textures.
  std::vector<TextureHeader> texture_headers;
  texture_headers.reserve(textures.size());
  for (auto* texture : textures) {
    TextureHeader texture_header = {};
    memcpy(texture_header.title, TextureHeader::kTitle, std::size(texture_header.title));

    texture_header.type = (uint8_t)texture->type;
    texture_header.size_x = (uint32_t)texture->size.x;
    texture_header.size_y = (uint32_t)texture->size.y;
    texture_header.wrap_mode_u = (uint8_t)texture->wrap_mode_u;
    texture_header.wrap_mode_v = (uint8_t)texture->wrap_mode_v;
    texture_header.min_filter_mode = (uint8_t)texture->min_filter;
    texture_header.mag_filter_mode = (uint8_t)texture->mag_filter;

    for (uint32_t i = 0; i < TextureHeader::kNameLength - 2; i++) {
      if (i >= texture->name.size())
        break;
      texture_header.name[i] = texture->name[i];
    }

    texture_headers.push_back(std::move(texture_header));
  }

  return {textures_header, std::move(texture_headers), std::move(textures)};
}

}  // namespace

bool SerializeScene(const Scene& scene, const std::string& path) {
  if (scene.valid)
    return false;

  FileHandle file = OpenFile(path);
  if (!Valid(file))
    return false;

  Header header = CalculateHeader(scene);
  auto [meshes_header, mesh_headers, meshes] = CalculateMeshes(scene);
  auto [textures_header, texture_headers, textures] = CalculateTextures(scene);

  // Add header.
  uint32_t current_offset = 0;

  header.next_mesh = sizeof(Header);
  current_offset = header.next_mesh;

  current_offset = DetermineMeshOffsets(current_offset, &meshes_header, &mesh_headers);

  WriteToFile(&file, &header, sizeof(Header));
  WriteToFile(&file, &meshes_header, sizeof(MeshesHeader));

  // Now that all the meshes are set, we can start sending off the data.
  for (uint32_t i = 0; i < mesh_headers.size(); i++) {
    auto& mesh_header = mesh_headers[i];

    mesh_header.data = current_offset;
    WriteToFile(&file, &mesh_header, sizeof(MeshHeader));

    current_offset += meshes[i]->vertices.size();
    current_offset += meshes[i]->indices.size() * sizeof(Mesh::IndexType);
  }

  // Actually write the data now.
  for (auto* mesh : meshes) {
    WriteToFile(&file, (void*)mesh->vertices.data(), mesh->vertices.size());
    WriteToFile(&file, (void*)mesh->indices.data(), mesh->indices.size() * sizeof(Mesh::IndexType));
  }

  return true;
}

// Read Scene --------------------------------------------------------------------------------------

namespace {

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

bool Verify(const TextureHeader& texture, uint32_t offset) {
  // clang-format off
  if (texture.title[0] != 'T' ||
      texture.title[1] != 'E' ||
      texture.title[2] != 'X') {
    LOG(Model, "Offset %u: Wrong texture header.", offset);
    return false;
  }
  // clang-format on

  if (texture.type >= (uint8_t)TextureType::kLast) {
    LOG(Model,
        "Offset %u: Wrong texture type: %u.",
        offset + (uint32_t)offsetof(TextureHeader, type),
        texture.type);
    return false;
  }

  if (texture.wrap_mode_u >= (uint8_t)TextureWrapMode::kLast ||
      texture.wrap_mode_v >= (uint8_t)TextureWrapMode::kLast) {
    LOG(Model,
        "Offset %u: Wrong UV wrap modes: (%u, %u).",
        offset + (uint32_t)offsetof(TextureHeader, wrap_mode_u),
        texture.wrap_mode_u,
        texture.wrap_mode_v);
    return false;
  }

  if (texture.min_filter_mode >= (uint8_t)TextureFilterMode::kLast ||
      texture.mag_filter_mode >= (uint8_t)TextureFilterMode::kLast) {
    LOG(Model,
        "Offset %u: Wrong filter modes: (%u, %u).",
        offset + (uint32_t)offsetof(TextureHeader, min_filter_mode),
        texture.min_filter_mode,
        texture.mag_filter_mode);
    return false;
  }

  return true;
}

}  // namespace

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
  printf("First mesh: %u\n", header->next_mesh);

  auto meshes = (const MeshesHeader*)(data + header->next_mesh);
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

}  // namespace gltf
}  // namespace rothko
