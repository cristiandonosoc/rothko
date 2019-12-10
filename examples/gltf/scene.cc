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

}  // namespace

bool SerializeScene(const Scene& scene, const std::string& path) {
  if (scene.valid)
    return false;

  FileHandle file = OpenFile(path);
  if (!Valid(file))
    return false;

  Header header = {};
  header.magic[0] = 'R';
  header.magic[1] = 'T';
  header.magic[2] = 'H';
  header.magic[3] = 'K';
  header.version = kVersion;
  header.meshes_header_offset = sizeof(Header);
  WriteToFile(&file, &header, sizeof(Header));

  // Meshes.
  MeshesHeader meshes_header = {};
  meshes_header.mesh_count = GetMeshesCount(scene.models);
  printf("MESH COUNT: %u.\n", meshes_header.mesh_count);
  WriteToFile(&file, &meshes_header, sizeof(MeshesHeader));

  // Go over the meshes.
  for (const auto& model : scene.models) {
    for (const auto& mesh : model->meshes) {
      MeshHeader mesh_header = {};
      mesh_header.vertex_type = (uint32_t)mesh->vertex_type;
      printf("Vertex Type %s: %u.\n", ToString(mesh->vertex_type), (uint32_t)mesh->vertex_type);
      mesh_header.vertex_count = mesh->vertex_count;
      printf("Vertex count: %u.\n", mesh_header.vertex_count);
      mesh_header.index_count = mesh->indices.size();
      printf("Index count: %u.\n", mesh_header.index_count);

      std::string mesh_name = mesh->name;
      if (mesh->name.size() > 63)
        mesh_name = mesh->name.substr(0, 63);
      uint32_t i = 0;
      for (char c : mesh_name) {
        mesh_header.name[i++] = c;
      }
      mesh_header.name[i] = 0;

      WriteToFile(&file, &mesh_header, sizeof(MeshHeader));
      WriteToFile(&file, mesh->vertices.data(), mesh->vertices.size());
      WriteToFile(&file, mesh->indices.data(), mesh_header.index_count * sizeof(Mesh::IndexType));
    }
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
  std::vector<uint8_t> data;
  if (!ReadWholeFile(path, &data))
    return {};

  // Verify header.
  Header* header = (Header*)data.data();
  if (!VerifyMagic(*header)) {
    printf("Invalid file header!\n");
    return {};
  }

  printf("Header version: %u\n", header->version);
  printf("Header mesh offset: %u\n", header->meshes_header_offset);

  MeshesHeader* meshes = (MeshesHeader*)(data.data() + header->meshes_header_offset);
  printf("Meshes count: %u\n", meshes->mesh_count);

  uint8_t* ptr = (uint8_t*)(meshes + 1);
  for (uint32_t i = 0; i < meshes->mesh_count; i++) {
    MeshHeader* mesh = (MeshHeader*)ptr;
    VertexType vertex_type = ToVertexType(mesh->vertex_type);
    printf("Mesh %u: Vertex Type %s: %u\n", i, ToString(vertex_type), mesh->vertex_type);
    printf("Mesh %u: Vertex Count: %u\n", i, mesh->vertex_count);
    printf("Mesh %u: Index Count: %u\n", i, mesh->index_count);

    char name[MeshHeader::kNameLength + 1] = {};
    memcpy(name, mesh->name, MeshHeader::kNameLength);
    printf("Mesh %u: Name: %s\n", i, name);


    break;
  }
  return {};
}

}  // namespace gltf
}  // namespace rothko
