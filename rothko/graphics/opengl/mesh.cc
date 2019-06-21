// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/opengl/mesh.h"

#include <GL/gl3w.h>
#include <stddef.h>

#include <atomic>

#include "rothko/graphics/common/mesh.h"
#include "rothko/graphics/opengl/renderer_backend.h"
#include "rothko/utils/logging.h"

namespace rothko {
namespace opengl {

// Stage Mesh ------------------------------------------------------------------

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
  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

void StageAttributes(Mesh* mesh) {
  if (mesh->vertex_type == VertexType::kDefault) {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3),
                          (void*)offsetof(VertexDefault, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3),
                          (void*)offsetof(VertexDefault, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2),
                          (void*)offsetof(VertexDefault, uv));
    return;
  }

  NOT_REACHED_MSG("Invalid vertex type: %s", ToString(mesh->vertex_type));
}

void StageVertices(Mesh* mesh, MeshHandles* handles) {
  glBindBuffer(GL_ARRAY_BUFFER, handles->vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size(),
                                mesh->vertices.data(),
                                GL_STATIC_DRAW);

  StageAttributes(mesh);
}

void StageIndices(Mesh* mesh, MeshHandles* handles) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size(),
                                        mesh->indices.data(),
                                        GL_STATIC_DRAW);
}

}  // namespace

bool OpenGLStageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  uint32_t uuid = GetNextMeshUUID();

  LOG(DEBUG, "Staging mesh %s (uuid %u).", mesh->name.c_str(), uuid);
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

  UnbindMeshHandles();

  opengl->loaded_meshes[uuid] = std::move(handles);
  mesh->uuid = uuid;

  return true;
}

// Unstage Mesh ----------------------------------------------------------------

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

}  // namespace opengl
}  // namespace rothko
