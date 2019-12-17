// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "scene.h"

#include <rothko/graphics/graphics.h>
#include <rothko/models/model.h>
#include <rothko/utils/file.h>

namespace rothko {
namespace gltf {

namespace {

constexpr uint32_t kVersion = 1;

uint32_t GetMeshesCount(const std::vector<std::unique_ptr<Model>>& models) {
  uint32_t count = 0;
  for (const auto& model : models) {
    count += model->meshes.size();
  }

  return count;
}

Header CalculateHeader(const Scene& scene) {
  (void)scene;
  Header header = {};
  header.magic[0] = 'R';
  header.magic[1] = 'T';
  header.magic[2] = 'H';
  header.magic[3] = 'K';
  header.version = kVersion;

  return header;
}

std::tuple<MeshesHeader, std::vector<MeshHeader>, std::vector<const Mesh*>>
CalculateMeshes(const Scene& scene) {
  MeshesHeader meshes_header = {};
  meshes_header.mesh_count = GetMeshesCount(scene.models);
  printf("MESH COUNT: %u.\n", meshes_header.mesh_count);

  /* WriteToFile(&file, &meshes_header, sizeof(MeshesHeader)); */

  // Get the meshes.
  std::vector<const Mesh*> meshes;
  for (const auto& model : scene.models) {
    for (const auto& mesh : model->meshes) {
      meshes.push_back(mesh.get());
    }
  }

  // Go over the meshes.
  std::vector<MeshHeader> mesh_headers;
  meshes.reserve(meshes.size());
  for (auto* mesh : meshes) {
    MeshHeader mesh_header = {};
    mesh_header.vertex_type = (uint32_t)mesh->vertex_type;
    mesh_header.vertex_count = mesh->vertex_count;
    mesh_header.index_count = mesh->indices.size();

    printf("Vertex Type %s: %u.\n", ToString(mesh->vertex_type), (uint32_t)mesh->vertex_type);
    printf("Vertex count: %u.\n", mesh_header.vertex_count);
    printf("Index count: %u.\n", mesh_header.index_count);

    std::string mesh_name = mesh->name;
    constexpr uint32_t kNameLen = MeshHeader::kNameLength - 1;
    if (mesh->name.size() > kNameLen)
      mesh_name = mesh->name.substr(0, kNameLen);

    uint32_t i = 0;
    for (char c : mesh_name) {
      mesh_header.name[i++] = c;
    }
    mesh_header.name[i] = 0;

    mesh_headers.push_back(std::move(mesh_header));

    /* WriteToFile(&file, &mesh_header, sizeof(MeshHeader)); */
    /* WriteToFile(&file, mesh->vertices.data(), mesh->vertices.size()); */
    /* WriteToFile(&file, mesh->indices.data(), mesh_header.index_count *
     * sizeof(Mesh::IndexType)); */
    /* } */
  }

  return {meshes_header, std::move(mesh_headers), std::move(meshes)};
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

  // Add header.
  uint32_t current_offset = 0;
  header.next_mesh = sizeof(Header);
  WriteToFile(&file, &header, sizeof(header));
  current_offset = header.next_mesh;

  meshes_header.next_mesh = current_offset + sizeof(MeshesHeader);
  WriteToFile(&file, &meshes_header, sizeof(meshes_header));
  current_offset = meshes_header.next_mesh;

  // Add the next header offsets.
  // All headers are next to each other. Data offset is calculated in another pass.
  for (auto& mesh_header : mesh_headers) {
    mesh_header.next_mesh = current_offset + sizeof(MeshHeader);
    current_offset = mesh_header.next_mesh;
  }

  // Now that all the meshes are set, we can start sending off the data.
  for (uint32_t i = 0; i < mesh_headers.size(); i++) {
    auto& mesh_header = mesh_headers[i];

    mesh_header.data = current_offset;
    WriteToFile(&file, &mesh_header, sizeof(mesh_header));

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

bool VerifyMagic(const Header& header) {
  // clang-format off
  if (header.magic[0] != 'R' ||
      header.magic[1] != 'T' ||
      header.magic[2] != 'H' ||
      header.magic[3] != 'K') {
    return false;
  }
  return true;
  // clang-format on
}

}  // namespace

Scene ReadScene(const std::string& path) {
  std::vector<uint8_t> file_data;
  if (!ReadWholeFile(path, &file_data))
    return {};

  printf("READING -----------------------------------------\n");

  // Verify header.
  const uint8_t* data = file_data.data();
  auto header = (const Header*)data;
  if (!VerifyMagic(*header)) {
    printf("Invalid file header!\n");
    return {};
  }

  printf("Header version: %u\n", header->version);
  printf("First mesh: %u\n", header->next_mesh);

  auto meshes = (const MeshesHeader*)(data + header->next_mesh);
  printf("Meshes count: %u\n", meshes->mesh_count);

  auto mesh = (const MeshHeader*)(data + meshes->next_mesh);
  for (uint32_t i = 0; i < meshes->mesh_count; i++) {
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
