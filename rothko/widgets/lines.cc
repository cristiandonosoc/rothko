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

std::unique_ptr<Shader> CreateLineShader(Renderer* renderer, const std::string& name) {
  ShaderConfig config = {};
  config.name = name;
  config.vertex_type = VertexType::k3dColor;

  auto vert_src = CreateVertexSource(kLineVertexShader);
  auto frag_src = CreateFragmentSource(kLineFragmentShader);


  return RendererStageShader(renderer, config, vert_src, frag_src);
}

// Init --------------------------------------------------------------------------------------------

namespace {

constexpr char kLineShaderName[] = "default-line-shader";

}  // namespace

bool Init(LineManager* lines, Renderer* renderer, const std::string& name, uint32_t line_count) {
  // See if we can get the shader.
  const Shader* shader = RendererGetShader(renderer, kLineShaderName);
  if (!shader) {
    // Otherwise we keep a global shader.
    static auto default_shader = CreateLineShader(renderer, kLineShaderName);
    if (!default_shader)
      return {};

    shader = default_shader.get();
  }

  return Init(lines, renderer, shader, name, line_count);
}

bool Init(LineManager* lines,
          Renderer* renderer,
          const Shader* shader,
          const std::string& name,
          uint32_t line_count) {
  ASSERT(!Valid(lines));
  lines->name = std::move(name);
  lines->shader = shader;

  // Each vertex is 2 vertices.
  // We asume one index per vertex. This might be less.
  uint32_t vertex_count = 2 * line_count * sizeof(Vertex3dColor);
  uint32_t index_count = 2 * line_count * sizeof(Mesh::IndexType);
  bool staged = StageWithCapacity(
      renderer, &lines->strip_mesh, VertexType::k3dColor, vertex_count, index_count);
  if (!staged)
    return false;

  lines->strip_mesh.name = StringPrintf("%s-mesh", lines->name.c_str());

  lines->render_command_ = {};
  /* ClearDepthTest(&lines->render_command_.flags); */
  lines->render_command_.mesh = &lines->strip_mesh;
  lines->render_command_.shader = lines->shader;
  lines->render_command_.primitive_type = PrimitiveType::kLineStrip;

  lines->staged = true;
  return true;
}

void Reset(LineManager* lines) {
  Reset(&lines->strip_mesh);
  lines->staged = false;
  lines->shape_count = 0;
  lines->render_command_.indices_count = 0;
}

bool Stage(LineManager* lines, Renderer* renderer) {
  if (lines->staged)
    return true;
  lines->staged = RendererUploadMeshRange(renderer, &lines->strip_mesh);
  return lines->staged;
}

RenderCommand GetRenderCommand(const LineManager& lines) {
  if (!lines.staged || lines.shape_count == 0)
    return Nop{};
  return lines.render_command_;
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

void PushLine(LineManager* lines, Vec3 from, Vec3 to, Color color) {
  Vertex3dColor vertices[2] = {
    CreateVertex(from, color),
    CreateVertex(to, color),
  };

  Mesh::IndexType base = lines->strip_mesh.vertex_count;
  Mesh::IndexType indices[3] = {base + 0, base + 1, line_strip::kPrimitiveReset};

  PushVertices(&lines->strip_mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&lines->strip_mesh, indices, ARRAY_SIZE(indices));

  lines->render_command_.indices_count += ARRAY_SIZE(indices);
  lines->staged = false;
  lines->shape_count++;
}

void PushCubeCenter(LineManager* lines, Vec3 c, Vec3 e, Color color) {
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

  Mesh::IndexType base = lines->strip_mesh.vertex_count;
  Mesh::IndexType indices[18] = {
    base + 0, base + 1, base + 3, base + 2, base + 0, base + 4, base + 5, base + 1,
    line_strip::kPrimitiveReset,

    base + 7, base + 3, base + 2, base + 6, base + 7, base + 5, base + 4, base + 6,
    line_strip::kPrimitiveReset,
  };

  PushVertices(&lines->strip_mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&lines->strip_mesh, indices, ARRAY_SIZE(indices));
  lines->render_command_.indices_count += ARRAY_SIZE(indices);

  lines->staged = false;
  lines->shape_count++;
}

// Push Ring ---------------------------------------------------------------------------------------

namespace {

constexpr int kRingVertexCount = 32;
constexpr float kRingAngle = kRadians360 / (float)kRingVertexCount;

}  // namespace

void PushRing(LineManager* lines, Vec3 center, Vec3 normal, float radius, Color color) {
  auto frame = GetAxisFrame(normal);
  PushRing(lines, center, frame, radius, color);
}

void PushRing(LineManager* lines, Vec3 center, const AxisFrame& frame, float radius, Color color) {
  Mat3 rotation = ToMat3(Rotate(frame.forward, kRingAngle));

  Vertex3dColor vertices[kRingVertexCount] = {};
  Mesh::IndexType indices[kRingVertexCount + 2] = {};
  Mesh::IndexType base = lines->strip_mesh.vertex_count;

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

  PushVertices(&lines->strip_mesh, vertices, std::size(vertices));
  PushIndices(&lines->strip_mesh, indices, std::size(indices));
  lines->render_command_.indices_count += std::size(indices);

  lines->staged = false;
  lines->shape_count++;
}

}  // namespace rothko
