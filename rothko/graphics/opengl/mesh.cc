// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/mesh.h"

#include <GL/gl3w.h>
#include <stddef.h>

#include <atomic>

#include "rothko/graphics/common/mesh.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/logging/logging.h"

namespace rothko {
namespace opengl {

// Stage Mesh --------------------------------------------------------------------------------------

namespace {

std::atomic<uint32_t> kNextMeshUUID = 1;
uint32_t GetNextMeshUUID() {
  uint32_t id = kNextMeshUUID++;
  ASSERT(id < UINT32_MAX);
  return id;
}

MeshHandles GenerateMeshHandles() {
  uint32_t buffers[2];
  glGenBuffers(ARRAY_SIZE(buffers), buffers);

  uint32_t vao;
  glGenVertexArrays(1, &vao);

  MeshHandles handles = {};
  handles.vbo = buffers[0];
  handles.ebo = buffers[1];
  handles.vao = vao;
  return handles;
}

void UnbindMeshHandles() {
  // Always unbind the VAO first, so that it doesn't overwrite.
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

// The static_asserts are here to ensure that if the vertices change, this code has to change too.
void StageAttributes(Mesh* mesh) {
  switch (mesh->vertex_type) {
    case VertexType::kDefault: {
      static_assert(sizeof(VertexDefault) == 32);
      GLsizei stride = sizeof(VertexDefault);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexDefault, pos));
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                            (void*)offsetof(VertexDefault, normal));
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexDefault, uv));
      return;
    }
    case VertexType::kColor: {
      static_assert(sizeof(VertexColor) == 24);
      GLsizei stride = sizeof(VertexColor);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexColor, pos));
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexColor, uv));
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
                            (void*)offsetof(VertexColor, color));
      return;
    }
    case VertexType::kImgui: {
      static_assert(sizeof(VertexImgui) == 20);
      GLsizei stride = sizeof(VertexImgui);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexImgui, pos));
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VertexImgui, uv));
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
                            (void*)offsetof(VertexImgui, color));
      return;
    }
    case VertexType::kLast:
      break;
  }

  NOT_REACHED_MSG("Invalid vertex type: %s", ToString(mesh->vertex_type));
}

void StageVertices(Mesh* mesh, MeshHandles* handles) {
  glBindBuffer(GL_ARRAY_BUFFER, handles->vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size(), mesh->vertices.data(), GL_STATIC_DRAW);
  StageAttributes(mesh);
}

void StageIndices(Mesh* mesh, MeshHandles* handles) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size(), mesh->indices.data(), GL_STATIC_DRAW);
}

}  // namespace

bool OpenGLStageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  uint32_t uuid = GetNextMeshUUID();

  auto it = opengl->loaded_meshes.find(uuid);
  if (it != opengl->loaded_meshes.end()) {
    LOG(ERROR, "Reloading mesh %s", mesh->name.c_str());
    return false;
  }

  // Always bind the VAO first, so that it doesn't overwrite.
  MeshHandles handles = GenerateMeshHandles();

  glBindVertexArray(handles.vao);
  StageVertices(mesh, &handles);
  StageIndices(mesh, &handles);
  glBindVertexArray(NULL);

  UnbindMeshHandles();

  LOG(DEBUG,
      "Staging mesh %s (uuid: %u, VAO: %u) [%u vertices (%zu bytes)] [%u indices (%zu bytes)]",
      mesh->name.c_str(), uuid, handles.vao,
      mesh->vertices_count, mesh->vertices.size(), mesh->indices_count, mesh->indices.size());

  opengl->loaded_meshes[uuid] = std::move(handles);
  mesh->uuid = uuid;

  return true;
}

// Unstage Mesh ------------------------------------------------------------------------------------

namespace {

void DeleteMeshHandles(MeshHandles* handles) {
  glDeleteBuffers(2, (GLuint*)handles);
  glDeleteVertexArrays(1, &handles->vao);
}

}  // namespace

void OpenGLUnstageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  uint32_t uuid = mesh->uuid.value;
  LOG(DEBUG, "Unstaging mesh %s (uuid %u).", mesh->name.c_str(), uuid);
  auto it = opengl->loaded_meshes.find(uuid);
  ASSERT(it != opengl->loaded_meshes.end());

  DeleteMeshHandles(&it->second);
  opengl->loaded_meshes.erase(it);
  mesh->uuid = 0;
}

// Upload Range ------------------------------------------------------------------------------------

namespace {

void VerifyBufferSize(GLenum target, uint32_t size, uint32_t offset) {
  GLint gl_size= 0;
  glGetBufferParameteriv(target, GL_BUFFER_SIZE, &gl_size);
  uint64_t buf_size = (uint64_t)gl_size;
  uint32_t total = size + offset;

  ASSERT_MSG(buf_size > (size + offset), "Buf size exceeded. %llu <= %u", buf_size, total);
}

}  // namespace


bool OpenGLUploadMeshRange(OpenGLRendererBackend* opengl, Mesh* mesh,
                           Int2 vertex_range, Int2 index_range) {
  uint64_t uuid = mesh->uuid.value;
  auto it = opengl->loaded_meshes.find(uuid);
  if (it == opengl->loaded_meshes.end()) {
    LOG(ERROR, "Uploading range on non-staged mesh %s", mesh->name.c_str());
    return false;
  }

  MeshHandles& handles = it->second;

  // Vertices.
  {
    uint32_t vertex_size = ToSize(mesh->vertex_type);
    uint32_t offset = vertex_range.x;
    uint32_t size = vertex_range.y;
    if (size == 0)
      size = mesh->vertices_count * vertex_size;

    glBindBuffer(GL_ARRAY_BUFFER, handles.vbo);
#if DEBUG_MODE
    VerifyBufferSize(GL_ARRAY_BUFFER, size, offset);
#endif
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh->vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, NULL);

    LOG(DEBUG, "Staged %u vertex bytes (%u vertices).", size, size / vertex_size);
  }

  // Indices.
  {
    uint32_t offset = index_range.x;
    uint32_t size = index_range.y;
    if (size == 0)
      size = mesh->indices_count * sizeof(Mesh::IndexType);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles.ebo);
#if DEBUG_MODE
    VerifyBufferSize(GL_ELEMENT_ARRAY_BUFFER, size, offset);
#endif
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, mesh->indices.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

    LOG(DEBUG, "Staged %u index bytes (%zu indices)", size, size / sizeof(Mesh::IndexType));
  }

  return true;
}

}  // namespace opengl
}  // namespace rothko
