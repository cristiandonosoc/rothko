// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/models/cube.h"

#include "rothko/graphics/color.h"

namespace rothko {

namespace {

Vertex3dUVColor CreateVertex(Vec3 pos, Vec2 uv, uint32_t color) {
  Vertex3dUVColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = color;

  return vertex;
}

}  // namespace

Mesh CreateCubeMesh(const std::string& name, Vec3 extents) {
  Mesh mesh = {};
  mesh.name = name;

  mesh.vertex_type = VertexType::k3dUVColor;
  Vertex3dUVColor vertices[] = {
      // X
      CreateVertex({-0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex({-0.5f, -0.5f,  0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex({-0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex({-0.5f,  0.5f, -0.5f}, {1, 0}, ToUint32(Color::Red())),

      CreateVertex({ 0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex({ 0.5f, -0.5f,  0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex({ 0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex({ 0.5f,  0.5f, -0.5f}, {1, 0}, ToUint32(Color::Red())),

      // Y
      CreateVertex({-0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex({ 0.5f, -0.5f, -0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex({ 0.5f, -0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex({-0.5f, -0.5f,  0.5f}, {1, 0}, ToUint32(Color::Red())),

      CreateVertex({-0.5f,  0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex({ 0.5f,  0.5f, -0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex({ 0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex({-0.5f,  0.5f,  0.5f}, {1, 0}, ToUint32(Color::Red())),

      // Z
      CreateVertex({-0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex({ 0.5f, -0.5f, -0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex({ 0.5f,  0.5f, -0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex({-0.5f,  0.5f, -0.5f}, {1, 0}, ToUint32(Color::Red())),

      CreateVertex({-0.5f, -0.5f,  0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex({ 0.5f, -0.5f,  0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex({ 0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex({-0.5f,  0.5f,  0.5f}, {1, 0}, ToUint32(Color::Red())),
  };

  // Apply the extents.
  for (Vertex3dUVColor& vertex : vertices) {
    vertex.pos *= extents;
  }

  Mesh::IndexType indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,

    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,

    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
  };

  PushVertices(&mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&mesh, indices, ARRAY_SIZE(indices));

  ASSERT_MSG(mesh.vertex_count == ARRAY_SIZE(vertices), "Count: %u", mesh.vertex_count);
  ASSERT(mesh.vertices.size() == sizeof(vertices));

  ASSERT_MSG(mesh.index_count == ARRAY_SIZE(indices), "Count: %u", mesh.index_count);
  ASSERT(mesh.indices.size() == sizeof(indices));

  return mesh;
}

}  // namespace rothko
