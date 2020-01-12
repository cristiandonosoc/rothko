// Copyright 2020, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/graphics/graphics.h>
#include <rothko/logging/logging.h>
#include <rothko/math/math.h>
#include <rothko/models/model.h>
#include <rothko/utils/file.h>

#include "scene.h"

namespace rothko {
namespace gltf {

namespace {

constexpr uint32_t kVersion = 1;

Header CreateMainHeader(const Scene& scene) {
  (void)scene;
  Header header = {};
  memcpy(header.title, Header::kTitle, std::size(header.title));
  header.version = kVersion;

  return header;
}

// Meshes ------------------------------------------------------------------------------------------

struct MeshesCalculationResult {
  MeshesHeader main_header;
  std::vector<MeshHeader> headers;
  std::vector<const Mesh*> meshes;
  uint32_t data_size;   // Total size of |meshes| data to be outputted. In Bytes.
};

MeshesCalculationResult CalculateMeshes(const Scene& scene) {
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

  // Go over the meshes. We assume they're going to be next to each other in the file.
  std::vector<MeshHeader> mesh_headers;
  meshes.reserve(meshes.size());

  uint32_t header_offset = 0;   // Where the next header is going to be.
  uint32_t data_offset = 0;     // Where the next mesh data is going to be.

  for (uint32_t i = 0; i < meshes.size(); i++) {
    auto* mesh = meshes[i];

    MeshHeader mesh_header = {};
    memcpy(mesh_header.title, MeshHeader::kTitle, std::size(mesh_header.title));

    mesh_header.vertex_type = (uint32_t)mesh->vertex_type;
    mesh_header.vertex_count = mesh->vertex_count;
    mesh_header.index_count = mesh->indices.size();

    printf("Vertex Type %s: %u.\n", ToString(mesh->vertex_type), (uint32_t)mesh->vertex_type);
    printf("Vertex count: %u.\n", mesh_header.vertex_count);
    printf("Index count: %u.\n", mesh_header.index_count);

    // Clamp the name.
    uint32_t name_length = Min((uint32_t)mesh->name.length(), MeshHeader::kNameLength - 1);
    memcpy(mesh_header.name, mesh->name.c_str(), name_length);

    // Mark where the next header is going to be.
    if (i < meshes.size() - 1)
      mesh_header.next_mesh = header_offset;
    header_offset += sizeof(MeshHeader);

    // Mark where the mesh data is going to be.
    mesh_header.data = data_offset;
    data_offset += mesh->vertices.size();
    data_offset += mesh->indices.size() * sizeof(Mesh::IndexType);

    mesh_headers.push_back(std::move(mesh_header));
  }

  MeshesCalculationResult result = {};
  result.main_header = std::move(meshes_header);
  result.headers = std::move(mesh_headers);
  result.meshes = std::move(meshes);
  result.data_size = data_offset;

  return result;
}

// Textures ----------------------------------------------------------------------------------------

struct TexturesCalculationResult {
  TexturesHeader main_header;
  std::vector<TextureHeader> headers;
  std::vector<const Texture*> textures;
  uint32_t data_size = 0;
};

TexturesCalculationResult CalculateTextures(const Scene& scene) {
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

  uint32_t header_offset = 0;   // Where the next header is going to be.
  uint32_t data_offset = 0;     // Where the next mesh data is going to be.

  for (uint32_t i = 0; i < textures.size(); i++) {
    const Texture* texture = textures[i];

    TextureHeader texture_header = {};
    memcpy(texture_header.title, TextureHeader::kTitle, std::size(texture_header.title));

    texture_header.type = (uint8_t)texture->type;
    texture_header.size_x = (uint32_t)texture->size.x;
    texture_header.size_y = (uint32_t)texture->size.y;
    texture_header.wrap_mode_u = (uint8_t)texture->wrap_mode_u;
    texture_header.wrap_mode_v = (uint8_t)texture->wrap_mode_v;
    texture_header.min_filter_mode = (uint8_t)texture->min_filter;
    texture_header.mag_filter_mode = (uint8_t)texture->mag_filter;

    // Clamp the name.
    uint32_t name_length = Min((uint32_t)texture->name.length(), MeshHeader::kNameLength - 1);
    memcpy(texture_header.name, texture->name.c_str(), name_length);

    if (i < textures.size() - 1)
      texture_header.next_texture = header_offset;
    header_offset += sizeof(TextureHeader);

    // Assumes all texture are together.
    texture_header.data = data_offset;
    data_offset += DataSize(*texture);

    texture_headers.push_back(std::move(texture_header));
  }

  TexturesCalculationResult result = {};
  result.main_header = std::move(textures_header);
  result.headers = std::move(texture_headers);
  result.textures = std::move(textures);
  result.data_size = data_offset;

  return result;
}

/* uint32_t DetermineTextureOffsets(uint32_t base_offset, TexturesHeader* textures_header, */
/*                                  std::vector<TexturesHeader>* texture_headers) { */
/*   textures_header-> */

/* } */

}  // namespace

bool SerializeScene(const Scene& scene, const std::string& path) {
  if (scene.valid)
    return false;

  FileHandle file = OpenFile(path);
  if (!Valid(file))
    return false;

  // Write the main header.
  Header header = CreateMainHeader(scene);
  header.meshes = sizeof(Header);
  header.textures = header.meshes + sizeof(MeshesHeader);
  WriteToFile(&file, &header, sizeof(Header));

  // Mesh headers start just after the main headers.
  auto meshes = CalculateMeshes(scene);
  meshes.main_header.next_mesh = header.textures + sizeof(TexturesHeader);
  WriteToFile(&file, &meshes.main_header, sizeof(MeshesHeader));

  // Texture headers start just after the mesh headers.
  auto textures = CalculateTextures(scene);
  textures.main_header.next_texture = meshes.main_header.next_mesh +
                                      meshes.headers.size() * sizeof(MeshHeader);
  WriteToFile(&file, &textures.main_header, sizeof(TexturesHeader));

  // Write the mesh headers.
  uint32_t mesh_data_start = textures.main_header.next_texture +
                             textures.headers.size() * sizeof(TextureHeader);
  for (uint32_t i = 0; i < meshes.headers.size(); i++) {
    MeshHeader* mesh_header = &meshes.headers[i];

    // Offset the headers and data.
    mesh_header->next_mesh += meshes.main_header.next_mesh;
    mesh_header->data += mesh_data_start;
    WriteToFile(&file, mesh_header, sizeof(MeshHeader));
  }

  // Write the texture headers.
  uint32_t texture_data_start = mesh_data_start + meshes.data_size;
  for (uint32_t i = 0; i < textures.headers.size(); i++) {
    TextureHeader* texture_header = &textures.headers[i];

    // Offset the headers and data.
    texture_header->next_texture = textures.main_header.next_texture;
    texture_header->data += texture_data_start;
    WriteToFile(&file, texture_header, sizeof(TextureHeader));
  }

  // Write the mesh data.

  // Actually write the data now.
  for (auto* mesh : meshes.meshes) {
    WriteToFile(&file, (void*)mesh->vertices.data(), mesh->vertices.size());
    WriteToFile(&file, (void*)mesh->indices.data(), mesh->indices.size() * sizeof(Mesh::IndexType));
  }

  for (auto* texture : textures.textures) {
    WriteToFile(&file, (void*)texture->data.get(), DataSize(*texture));
  }

  /* // Meshes start directly after main headers. */

  /* meshes_header.next_mesh = data_offset; */
  /* WriteToFile(&file, &meshes_header, sizeof(MeshesHeader)); */



  /* // Write the main headers. */
  /* WriteToFile(&file, &meshes_header, sizeof(MeshesHeader)); */

  /* uint32_t mesh_data_offset = data_offset; */
  /* for (uint32_t i = 0; i < mesh_headers.size(); i++) { */
  /*   auto& mesh_header = mesh_headers[i]; */

  /*   mesh_header.data = mesh_data_offset; */
  /*   WriteToFile(&file, &mesh_header, sizeof(MeshHeader)); */

  /*   mesh_data_offset += meshes[i]->vertices.size(); */
  /*   mesh_data_offset += meshes[i]->indices.size() * sizeof(Mesh::IndexType); */
  /* } */

  return true;
}

}  // namespace gltf
}  // namespace rothko
