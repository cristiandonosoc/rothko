// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/mesh.h"

#include <GL/gl3w.h>
#include <inttypes.h>
#include <stddef.h>

#include <atomic>

#include "rothko/graphics/mesh.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/graphics/opengl/utils.h"
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
    case VertexType::k3dNormalUV: {
      using VERT = Vertex3dNormalUV;
      static_assert(sizeof(VERT) == 32);
      GLsizei stride = sizeof(VERT);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, pos));
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, normal));
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, uv));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      return;
    }
    case VertexType::k2dUVColor: {
      using VERT = Vertex2dUVColor;
      static_assert(sizeof(VERT) == 20);
      GLsizei stride = sizeof(VERT);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, pos));
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, uv));
      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(VERT, color));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      return;
    }
    case VertexType::k3dColor: {
      using VERT = Vertex3dColor;
      static_assert(sizeof(VERT) == 16);
      GLsizei stride = sizeof(VERT);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, pos));
      glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(VERT, color));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      return;
    }
    case VertexType::k3dUV: {
      using VERT = Vertex3dUV;
      static_assert(sizeof(VERT) == 20);
      GLsizei stride = sizeof(VERT);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, pos));
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, uv));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      return;
    }
    case VertexType::k3dUVColor: {
      using VERT = Vertex3dUVColor;
      static_assert(sizeof(VERT) == 24);
      GLsizei stride = sizeof(VERT);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, pos));
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, uv));
      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(VERT, color));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      return;
    }
    case VertexType::k3dNormalTangentUV: {
      using VERT = Vertex3dNormalTangentUV;
      static_assert(sizeof(VERT) == 48);
      GLsizei stride = sizeof(VERT);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, pos));
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, normal));
      glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, tangent));
      glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(VERT, uv));
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glEnableVertexAttribArray(3);
      glEnableVertexAttribArray(4);
      return;
    }
    case VertexType::kLast: break;
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
  ASSERT_MSG(!Staged(*mesh), "Mesh \"%s\" already staged.", mesh->name.c_str());
  uint32_t uuid = GetNextMeshUUID();

  auto it = opengl->loaded_meshes.find(uuid);
  if (it != opengl->loaded_meshes.end()) {
    ERROR(OpenGL, "Reloading mesh \"%s\"", mesh->name.c_str());
    return false;
  }

  // Always bind the VAO first, so that it doesn't overwrite.
  MeshHandles handles = GenerateMeshHandles();

  glBindVertexArray(handles.vao);
  StageVertices(mesh, &handles);
  StageIndices(mesh, &handles);
  glBindVertexArray(NULL);

  UnbindMeshHandles();

  LOG(OpenGL,
      "Staging mesh %s (uuid: %u, VAO: %u) [%u vertices (%zu bytes)] [%u indices (%zu bytes)]",
      mesh->name.c_str(), uuid, handles.vao,
      mesh->vertex_count, mesh->vertices.size(), mesh->index_count, mesh->indices.size());

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
  auto it = opengl->loaded_meshes.find(uuid);
  ASSERT(it != opengl->loaded_meshes.end());

  DeleteMeshHandles(&it->second);
  opengl->loaded_meshes.erase(it);
  mesh->uuid = 0;
}

// Upload Range ------------------------------------------------------------------------------------

namespace {

void VerifyBufferSize(Mesh* mesh, GLenum target, uint32_t size, uint32_t offset) {
  GLint gl_size= 0;
  glGetBufferParameteriv(target, GL_BUFFER_SIZE, &gl_size);
  uint64_t buf_size = (uint64_t)gl_size;
  uint32_t total = size + offset;

  ASSERT_MSG(buf_size >= (size + offset), "Mesh %s (%s): Buf size exceeded. %" PRIu64 " <= %u",
             mesh->name.c_str(), ToString(target), buf_size, total);
}

}  // namespace


bool OpenGLUploadMeshRange(OpenGLRendererBackend* opengl, Mesh* mesh,
                           Int2 vertex_range, Int2 index_range) {
  uint64_t uuid = mesh->uuid.value;
  auto it = opengl->loaded_meshes.find(uuid);
  if (it == opengl->loaded_meshes.end()) {
    ERROR(OpenGL, "Uploading range on non-staged mesh %s", mesh->name.c_str());
    return false;
  }

  MeshHandles& handles = it->second;

  // Vertices.
  {
    uint32_t vertex_size = ToSize(mesh->vertex_type);
    uint32_t offset = vertex_range.x;
    uint32_t size = vertex_range.y;
    if (size == 0)
      size = mesh->vertex_count * vertex_size;

    glBindBuffer(GL_ARRAY_BUFFER, handles.vbo);
#if DEBUG_MODE
    VerifyBufferSize(mesh, GL_ARRAY_BUFFER, size, offset);
#endif
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh->vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
  }

  // Indices.
  {
    uint32_t offset = index_range.x;
    uint32_t size = index_range.y;
    if (size == 0)
      size = mesh->index_count * sizeof(Mesh::IndexType);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles.ebo);
#if DEBUG_MODE
    VerifyBufferSize(mesh, GL_ELEMENT_ARRAY_BUFFER, size, offset);
#endif
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, mesh->indices.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
  }

  return true;
}

}  // namespace opengl
}  // namespace rothko
