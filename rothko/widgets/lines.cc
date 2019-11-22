// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/widgets/lines.h"

#include "rothko/utils/strings.h"

namespace rothko {

// CreateLineShader --------------------------------------------------------------------------------

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

}  // namespace

std::unique_ptr<Shader> CreateLineShader(Renderer* renderer) {
  ShaderConfig config = {};
  config.name = "line-shader";
  config.vertex_type = VertexType::k3dColor;

  auto vert_src = CreateVertexSource(kLineVertexShader);
  auto frag_src = CreateFragmentSource(kLineFragmentShader);


  return RendererStageShader(renderer, config, vert_src, frag_src);
}

// Init --------------------------------------------------------------------------------------------

bool Init(LineManager* line_manager,
          Renderer* renderer,
          const Shader* shader,
          const std::string& name,
          uint32_t line_count) {
  ASSERT(!Valid(line_manager));
  line_manager->name = std::move(name);
  line_manager->shader = shader;

  // Each vertex is 2 vertices.
  // We asume one index per vertex. This might be less.
  uint32_t vertex_count = 2 * line_count * sizeof(Vertex3dColor);
  uint32_t index_count = 2 * line_count * sizeof(Mesh::IndexType);
  bool staged = StageWithCapacity(
      renderer, &line_manager->strip_mesh, VertexType::k3dColor, vertex_count, index_count);
  if (!staged)
    return false;

  line_manager->strip_mesh.name = StringPrintf("%s-mesh", line_manager->name.c_str());

  line_manager->render_command_ = {};
  line_manager->render_command_.mesh = &line_manager->strip_mesh;
  line_manager->render_command_.shader = line_manager->shader;
  line_manager->render_command_.primitive_type = PrimitiveType::kLineStrip;

  line_manager->staged = true;
  return true;
}

void Reset(LineManager* line_manager) {
  Reset(&line_manager->strip_mesh);
}

bool Stage(LineManager* line_manager, Renderer* renderer) {
  if (line_manager->staged)
    return true;
  line_manager->staged = RendererUploadMeshRange(renderer, &line_manager->strip_mesh);
  return line_manager->staged;
}

RenderCommand GetRenderCommand(const LineManager& line_manager) {
  if (!line_manager.staged || line_manager.shape_count == 0)
    return Nop{};
  return line_manager.render_command_;
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

  line_manager->render_command_.indices_count += ARRAY_SIZE(indices);
  line_manager->staged = false;
  line_manager->shape_count++;
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
  line_manager->render_command_.indices_count += ARRAY_SIZE(indices);

  line_manager->staged = false;
  line_manager->shape_count++;
}

// Push Ring ---------------------------------------------------------------------------------------

namespace {

constexpr int kRingVertexCount = 32;
constexpr float kRingAngle = kRadians360 / (float)kRingVertexCount;

}  // namespace

void PushRing(LineManager* line_manager, Vec3 center, Vec3 normal, float radius, Color color) {
  auto frame = GetAxisFrame(normal);
  PushRing(line_manager, center, frame, radius, color);
}

void PushRing(
    LineManager* line_manager, Vec3 center, const AxisFrame& frame, float radius, Color color) {
  Mat3 rotation = ToMat3(Rotate(frame.forward, kRingAngle));

  Vertex3dColor vertices[kRingVertexCount] = {};
  Mesh::IndexType indices[kRingVertexCount + 2] = {};
  Mesh::IndexType base = line_manager->strip_mesh.vertex_count;

  Vec3 p = frame.up * radius;
  for (int i = 0; i < kRingVertexCount; i++) {
    vertices[i] = CreateVertex(center + p, color);
    indices[i] = base + i;

    // Rotate the point.
    p = rotation * p;
    /* Vec3 p = {radius * Cos(i * kRingAngle), 0, radius * Sin(i * kRingAngle)}; */
    /* vertices[i] = CreateVertex(center + p, color); */
  }
  indices[kRingVertexCount] = base;  // Loop back to the beginning of the ring.
  indices[kRingVertexCount + 1] = line_strip::kPrimitiveReset;

  PushVertices(&line_manager->strip_mesh, vertices, std::size(vertices));
  PushIndices(&line_manager->strip_mesh, indices, std::size(indices));
  line_manager->render_command_.indices_count += std::size(indices);

  line_manager->staged = false;
  line_manager->shape_count++;
}

}  // namespace rothko
