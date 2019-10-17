// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/models/cube.h"

#include "rothko/graphics/color.h"

namespace rothko {

// Helpers -----------------------------------------------------------------------------------------

namespace {

std::vector<Mesh::IndexType> GetIndices() {
  std::vector<Mesh::IndexType> indices = {
    // X
    0, 1, 2, 2, 3, 0,
    /* 4, 5, 6, 6, 7, 4, */
    6, 5, 4, 4, 7, 6,

    // Y
    8, 9, 10, 10, 11, 8,
    /* 12, 13, 14, 14, 15, 12, */
    14, 13, 12, 12, 15, 14,

    // Z
    18, 17, 16, 16, 19, 18,
    20, 21, 22, 22, 23, 20,

  };

  return indices;
};

}  // namespace

// 3d ----------------------------------------------------------------------------------------------

namespace {

Vertex3d CreateVertex_3d(Vec3 pos) {
  Vertex3d vertex = {};
  vertex.pos = pos;

  return vertex;
}

Mesh CreateCubeMesh_3d(const std::string& name, Vec3 extents) {
  Mesh mesh = {};
  mesh.name = name;

  mesh.vertex_type = VertexType::k3d;

  // clang-format off
  Vertex3d vertices[] = {
      // X
      CreateVertex_3d({-0.5f, -0.5f, -0.5f}),
      CreateVertex_3d({-0.5f, -0.5f,  0.5f}),
      CreateVertex_3d({-0.5f,  0.5f,  0.5f}),
      CreateVertex_3d({-0.5f,  0.5f, -0.5f}),

      CreateVertex_3d({ 0.5f, -0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f, -0.5f,  0.5f}),
      CreateVertex_3d({ 0.5f,  0.5f,  0.5f}),
      CreateVertex_3d({ 0.5f,  0.5f, -0.5f}),

      // Y
      CreateVertex_3d({-0.5f, -0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f, -0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f, -0.5f,  0.5f}),
      CreateVertex_3d({-0.5f, -0.5f,  0.5f}),

      CreateVertex_3d({-0.5f,  0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f,  0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f,  0.5f,  0.5f}),
      CreateVertex_3d({-0.5f,  0.5f,  0.5f}),

      // Z
      CreateVertex_3d({-0.5f, -0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f, -0.5f, -0.5f}),
      CreateVertex_3d({ 0.5f,  0.5f, -0.5f}),
      CreateVertex_3d({-0.5f,  0.5f, -0.5f}),

      CreateVertex_3d({-0.5f, -0.5f,  0.5f}),
      CreateVertex_3d({ 0.5f, -0.5f,  0.5f}),
      CreateVertex_3d({ 0.5f,  0.5f,  0.5f}),
      CreateVertex_3d({-0.5f,  0.5f,  0.5f}),
  };
  // clang-format on

  // Apply the extents.
  for (Vertex3d& vertex : vertices) {
    vertex.pos *= extents;
  }

  PushVertices(&mesh, vertices, ARRAY_SIZE(vertices));
  /* PushIndices(&mesh, indices, ARRAY_SIZE(indices)); */
  mesh.indices = GetIndices();

  ASSERT_MSG(mesh.vertex_count == ARRAY_SIZE(vertices), "Count: %u", mesh.vertex_count);
  return mesh;
}

}  // namespace

// 3dNormal ----------------------------------------------------------------------------------------

namespace {

Vertex3dNormal CreateVertex_3dNormal(Vec3 pos, Vec3 normal) {
  Vertex3dNormal vertex = {};

  vertex.pos = pos;
  vertex.normal = normal;

  return vertex;
}

Mesh CreateCubeMesh_3dNormal(const std::string& name, Vec3 extents) {
  Mesh mesh = {};
  mesh.name = name;

  mesh.vertex_type = VertexType::k3dNormal;
  // clang-format off
  Vertex3dNormal vertices[] = {
      // X
      CreateVertex_3dNormal({-0.5f, -0.5f, -0.5f}, {-1,  0,  0}),
      CreateVertex_3dNormal({-0.5f, -0.5f,  0.5f}, {-1,  0,  0}),
      CreateVertex_3dNormal({-0.5f,  0.5f,  0.5f}, {-1,  0,  0}),
      CreateVertex_3dNormal({-0.5f,  0.5f, -0.5f}, {-1,  0,  0}),

      CreateVertex_3dNormal({ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}),
      CreateVertex_3dNormal({ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}),
      CreateVertex_3dNormal({ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}),
      CreateVertex_3dNormal({ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}),

      // Y
      CreateVertex_3dNormal({-0.5f, -0.5f, -0.5f}, { 0, -1,  0}),
      CreateVertex_3dNormal({ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}),
      CreateVertex_3dNormal({ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}),
      CreateVertex_3dNormal({-0.5f, -0.5f,  0.5f}, { 0, -1,  0}),

      CreateVertex_3dNormal({-0.5f,  0.5f, -0.5f}, { 0,  1,  0}),
      CreateVertex_3dNormal({ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}),
      CreateVertex_3dNormal({ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}),
      CreateVertex_3dNormal({-0.5f,  0.5f,  0.5f}, { 0,  1,  0}),

      // Z
      CreateVertex_3dNormal({-0.5f, -0.5f, -0.5f}, { 0,  0, -1}),
      CreateVertex_3dNormal({ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}),
      CreateVertex_3dNormal({ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}),
      CreateVertex_3dNormal({-0.5f,  0.5f, -0.5f}, { 0,  0, -1}),

      CreateVertex_3dNormal({-0.5f, -0.5f,  0.5f}, { 0,  0,  1}),
      CreateVertex_3dNormal({ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}),
      CreateVertex_3dNormal({ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}),
      CreateVertex_3dNormal({-0.5f,  0.5f,  0.5f}, { 0,  0,  1}),
  };
  // clang-format on

  // Apply the extents.
  for (Vertex3dNormal& vertex : vertices) {
    vertex.pos *= extents;
  }

  PushVertices(&mesh, vertices, ARRAY_SIZE(vertices));
  mesh.indices = GetIndices();

  ASSERT_MSG(mesh.vertex_count == ARRAY_SIZE(vertices), "Count: %u", mesh.vertex_count);

  return mesh;
}

}  // namespace

// 3dUVColor ---------------------------------------------------------------------------------------

namespace {

Vertex3dUVColor CreateVertex_3dUVColor(Vec3 pos, Vec2 uv, uint32_t color) {
  Vertex3dUVColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = color;

  return vertex;
}

Mesh CreateCubeMesh_3dUVColor(const std::string& name, Vec3 extents) {
  Mesh mesh = {};
  mesh.name = name;

  mesh.vertex_type = VertexType::k3dUVColor;

  // clang-format off
  Vertex3dUVColor vertices[] = {
      // X
      CreateVertex_3dUVColor({-0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex_3dUVColor({-0.5f, -0.5f,  0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex_3dUVColor({-0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex_3dUVColor({-0.5f,  0.5f, -0.5f}, {1, 0}, ToUint32(Color::Red())),

      CreateVertex_3dUVColor({ 0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex_3dUVColor({ 0.5f, -0.5f,  0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex_3dUVColor({ 0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex_3dUVColor({ 0.5f,  0.5f, -0.5f}, {1, 0}, ToUint32(Color::Red())),

      // Y
      CreateVertex_3dUVColor({-0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex_3dUVColor({ 0.5f, -0.5f, -0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex_3dUVColor({ 0.5f, -0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex_3dUVColor({-0.5f, -0.5f,  0.5f}, {1, 0}, ToUint32(Color::Red())),

      CreateVertex_3dUVColor({-0.5f,  0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex_3dUVColor({ 0.5f,  0.5f, -0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex_3dUVColor({ 0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex_3dUVColor({-0.5f,  0.5f,  0.5f}, {1, 0}, ToUint32(Color::Red())),

      // Z
      CreateVertex_3dUVColor({-0.5f, -0.5f, -0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex_3dUVColor({ 0.5f, -0.5f, -0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex_3dUVColor({ 0.5f,  0.5f, -0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex_3dUVColor({-0.5f,  0.5f, -0.5f}, {1, 0}, ToUint32(Color::Red())),

      CreateVertex_3dUVColor({-0.5f, -0.5f,  0.5f}, {0, 0}, ToUint32(Color::Blue())),
      CreateVertex_3dUVColor({ 0.5f, -0.5f,  0.5f}, {0, 1}, ToUint32(Color::Green())),
      CreateVertex_3dUVColor({ 0.5f,  0.5f,  0.5f}, {1, 1}, ToUint32(Color::White())),
      CreateVertex_3dUVColor({-0.5f,  0.5f,  0.5f}, {1, 0}, ToUint32(Color::Red())),
  };
  // clang-format on

  // Apply the extents.
  for (Vertex3dUVColor& vertex : vertices) {
    vertex.pos *= extents;
  }

  PushVertices(&mesh, vertices, ARRAY_SIZE(vertices));
  mesh.indices = GetIndices();

  ASSERT_MSG(mesh.vertex_count == ARRAY_SIZE(vertices), "Count: %u", mesh.vertex_count);

  return mesh;
}

}  // namespace

// CreateCubeMesh ----------------------------------------------------------------------------------

Mesh CreateCubeMesh(VertexType vertex_type, const std::string& name, Vec3 extents) {
  switch (vertex_type) {
    case VertexType::k2dUVColor: return {};
    case VertexType::k3d: return CreateCubeMesh_3d(name, extents);
    case VertexType::k3dColor: return {};
    case VertexType::k3dNormal: return CreateCubeMesh_3dNormal(name, extents);
    case VertexType::k3dNormalUV: return {};
    case VertexType::k3dUV: return {};
    case VertexType::k3dUVColor: return CreateCubeMesh_3dUVColor(name, extents);
    case VertexType::k3dNormalTangentUV: return {};
    case VertexType::kLast: return {};
  }

  NOT_REACHED();
  return {};
};

}  // namespace rothko
