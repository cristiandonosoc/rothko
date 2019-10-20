// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/scene/lines.h"

#include "rothko/utils/strings.h"

namespace rothko {

// Init --------------------------------------------------------------------------------------------

namespace {

constexpr char kLineVertexShader[] = R"(
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 f_color;

void main() {
  gl_Position = camera_proj * camera_view * vec4(in_pos, 1);
  f_color = in_color;
}
)";

constexpr char kLineFragmentShader[] = R"(
in vec4 f_color;

out vec4 out_color;

void main() {
  out_color = f_color;
}
)";

bool InitShader(Renderer* renderer, LineManager* line_manager) {
  Shader* shader = &line_manager->shader;
  shader->name = StringPrintf("%s-shader", line_manager->name.c_str());
  shader->vertex_type = VertexType::k3dColor;
  shader->vert_src = CreateVertexSource(kLineVertexShader);
  shader->frag_src = CreateFragmentSource(kLineFragmentShader);

  return RendererStageShader(renderer, shader);
}

}  // namespace

bool Init(Renderer* renderer, LineManager* line_manager, std::string name, uint32_t line_count) {
  ASSERT(!Valid(line_manager));
  line_manager->name = std::move(name);

  if (!InitShader(renderer, line_manager))
    return false;

  // Each vertex is 2 vertices.
  // We asume one index per vertex. This might be less.
  uint32_t vertex_count = 2 * line_count * sizeof(Vertex3dColor);
  uint32_t index_count = 2 * line_count * sizeof(Mesh::IndexType);
  bool staged = StageWithCapacity(renderer, &line_manager->strip_mesh, VertexType::k3dColor,
                                  vertex_count, index_count);
  if (!staged)
    return false;

  line_manager->strip_mesh.name = StringPrintf("%s-mesh", line_manager->name.c_str());

  line_manager->render_command = {};
  line_manager->render_command.mesh = &line_manager->strip_mesh;
  line_manager->render_command.shader = &line_manager->shader;
  line_manager->render_command.primitive_type = PrimitiveType::kLineStrip;

  line_manager->staged = true;
  return true;
}

void Reset(LineManager* line_manager) {
  Reset(&line_manager->strip_mesh);
}

bool Stage(Renderer* renderer, LineManager* line_manager) {
  if (line_manager->staged)
    return true;
  return RendererUploadMeshRange(renderer, &line_manager->strip_mesh);
}

// Push Lines --------------------------------------------------------------------------------------

namespace {

inline Vertex3dColor CreateVertex(Vec3 pos, Color color) {
  Vertex3dColor vertex = {};
  vertex.pos = pos;
  vertex.color = ToUint32(color);

  return vertex;
}

}  // namespace

void PushLine(LineManager* line_manager, Vec3 from, Vec3 to, Color color) {
  Vertex3dColor vertices[2] = {
    CreateVertex(from, color),
    CreateVertex(to, color),
  };

  Mesh::IndexType base = line_manager->strip_mesh.vertex_count;
  Mesh::IndexType indices[3] = {base + 0, base + 1, line_strip::kPrimitiveReset};

  PushVertices(&line_manager->strip_mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&line_manager->strip_mesh, indices, ARRAY_SIZE(indices));

  /* line_manager->render_command.indices_size += ARRAY_SIZE(indices); */
  line_manager->staged = false;
}

void PushCubeCenter(LineManager* line_manager, Vec3 c, Vec3 e, Color color) {
  Vertex3dColor vertices[8] = {
      CreateVertex(c + Vec3{-e.x, -e.y, -e.z}, color),
      CreateVertex(c + Vec3{-e.x, -e.y, e.z}, color),
      CreateVertex(c + Vec3{-e.x, e.y, -e.z}, color),
      CreateVertex(c + Vec3{-e.x, e.y, e.z}, color),
      CreateVertex(c + Vec3{e.x, -e.y, -e.z}, color),
      CreateVertex(c + Vec3{e.x, -e.y, e.z}, color),
      CreateVertex(c + Vec3{e.x, e.y, -e.z}, color),
      CreateVertex(c + Vec3{e.x, e.y, e.z}, color),
  };

  Mesh::IndexType base = line_manager->strip_mesh.vertex_count;
  Mesh::IndexType indices[18] = {
    base + 0, base + 1, base + 3, base + 2, base + 0, base + 4, base + 5, base + 1,
    line_strip::kPrimitiveReset,

    base + 7, base + 3, base + 2, base + 6, base + 7, base + 5, base + 4, base + 6,
    line_strip::kPrimitiveReset,
  };

  PushVertices(&line_manager->strip_mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&line_manager->strip_mesh, indices, ARRAY_SIZE(indices));
  /* line_manager->render_command.indices_size += ARRAY_SIZE(indices); */
  line_manager->render_command.indices_count = ARRAY_SIZE(indices);

  line_manager->staged = false;
}

}  // namespace rothko
