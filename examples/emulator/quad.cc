// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "quad.h"

namespace rothko {

bool Init(Renderer* renderer, QuadManager* quads, const QuadManagerConfig& config) {
  quads->name = config.name;
  quads->capacity = config.capacity;

  quads->mesh.name = config.name;
  quads->mesh.vertex_type = VertexType::k3dUVColor;

  // Each quad entry is reflected into 4 vertices.
  quads->mesh.vertices = std::vector<uint8_t>(4 * sizeof(Vertex3dUVColor) * config.capacity);
  quads->mesh.vertex_count = 4 * config.capacity;

  LOG(App, "Mesh size: %zu", quads->mesh.vertices.size());


  // Each quad is 6 vertices.
  quads->mesh.indices = std::vector<uint8_t>(6 * sizeof(Mesh::IndexType) * config.capacity);
  quads->mesh.index_count = 6 * config.capacity;

  if (!RendererStageMesh(renderer, &quads->mesh))
    return false;

  // Now that they're staged, we clear the buffer.
  Reset(&quads->mesh);

  quads->staged = true;
  return true;
}

void Reset(QuadManager* quads) {
  Reset(&quads->mesh);
  quads->staged = true;
  quads->size = 0;
}

// Push --------------------------------------------------------------------------------------------

namespace {

Vertex3dUVColor CreateVertex(Vec3 pos, Vec2 uv, Color color) {
  Vertex3dUVColor vertex = {};
  vertex.pos = pos;
  vertex.uv = uv;
  vertex.color = ToUint32(color);

  return vertex;
}

bool AreEqual(const RenderMesh& render_mesh, const QuadEntry& entry) {
  return render_mesh.shader == entry.shader &&
         render_mesh.textures[0] == entry.texture &&
         render_mesh.vert_ubo_data == entry.vert_ubo &&
         render_mesh.frag_ubo_data == entry.frag_ubo;
}

}  // namespace

void Push(QuadManager* quads, const QuadEntry& entry) {
  ASSERT(Valid(*quads));
  ASSERT_MSG((int)quads->render_commands.size() <= quads->capacity, "%zu <= %d",
             quads->render_commands.size(), quads->capacity);
  ASSERT(entry.shader);
  ASSERT(entry.texture);

  // Push in the vertex/index data.
  Vertex3dUVColor vertices[] = {
      /* CreateVertex({base.x, base.y, 0}, uv_base, colors::kWhite), */
      CreateVertex(entry.from_pos, entry.from_uv, entry.color),
      /* CreateVertex({base.x, base.y + size.y, 0}, uv_base + Vec2{kUVOffset.x, 0.0f}, colors::kWhite), */
      CreateVertex({entry.from_pos.x, entry.from_pos.y, entry.to_pos.z},
                   {entry.to_uv.u, entry.from_uv.v},
                   entry.color),
      /* CreateVertex({base.x + size.x, base.y, 0}, uv_base + Vec2{0, kUVOffset.y}, colors::kWhite), */
      CreateVertex({entry.to_pos.x, entry.to_pos.y, entry.from_pos.z},
                   {entry.from_uv.u, entry.to_uv.v},
                   entry.color),
      /* CreateVertex({base.x + size.x, base.y + size.y, 0}, uv_base + kUVOffset, colors::kWhite), */
      CreateVertex(entry.to_pos, entry.to_uv, entry.color),
  };

  Mesh::IndexType base = quads->mesh.vertex_count;
  Mesh::IndexType indices[] = {
    base + 0, base + 1, base + 2, base + 2, base + 1, base + 3,
  };

  PushVertices(&quads->mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&quads->mesh, indices, ARRAY_SIZE(indices));

  // Push in the render command.
  if (quads->render_commands.empty() ||
      !AreEqual(quads->render_commands.back().GetRenderMesh(), entry)) {
    RenderMesh render_mesh = {};
    render_mesh.mesh = &quads->mesh;
    render_mesh.shader = entry.shader;
    render_mesh.primitive_type = PrimitiveType::kTriangles;
    render_mesh.textures.push_back(entry.texture);
    render_mesh.indices_offset = quads->index_offset;
    render_mesh.indices_size = 6;
    render_mesh.vert_ubo_data = entry.vert_ubo;
    render_mesh.frag_ubo_data = entry.frag_ubo;

    quads->index_offset += 6;

    quads->render_commands.push_back(std::move(render_mesh));
  } else {
    // We can expand the previous render command.
    auto& render_mesh = quads->render_commands.back().GetRenderMesh();
    render_mesh.indices_size += 6;
    quads->index_offset += 6;
  }

  quads->staged = false;
  quads->size++;
}

void Stage(Renderer* renderer, QuadManager* quads) {
  if (quads->staged)
    return;

  if (quads->render_commands.empty()) {
    quads->staged = true;
    return;
  }

  bool res = RendererUploadMeshRange(renderer, &quads->mesh);
  ASSERT(res);

  quads->staged = true;
}

}  // namespace rothko
