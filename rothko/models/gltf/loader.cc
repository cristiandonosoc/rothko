// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/models/gltf/loader.h"

#include <third_party/tiny_gltf/tiny_gltf.h>

#include <map>
#include <set>

#include "rothko/graphics/graphics.h"
#include "rothko/logging/logging.h"
#include "rothko/models/model.h"
#include "rothko/scene/scene_graph.h"
#include "rothko/utils/strings.h"

namespace rothko {
namespace gltf {

namespace {

struct NodeContext {
  const SceneNode* scene_node = nullptr;
  uint32_t index = UINT32_MAX;
  uint32_t parent_index = UINT32_MAX;
};

struct ProcessingContext {
  Model model;

  std::unique_ptr<SceneGraph> scene_graph;
  std::vector<NodeContext> scene_nodes;

  // Set of resources we have already seen. These are NOT the rothko resources that are outputted.
  // Those are moved into the model at the end of processing. We use memory stability to make sure
  // that the binding is correct.
  std::set<int> processed_node_meshes;

  // The uniqueness of the mesh (which corresponds to the glTF primitive) is ensured by the
  // |processed_node_meshes| set.
  std::vector<std::unique_ptr<Mesh>> meshes;

  std::map<int, std::unique_ptr<Texture>> textures;
  std::map<int, std::unique_ptr<Material>> materials;
};

enum class BufferViewTarget : int {
  kArrayBuffer = 34962,
  kElementArrayBuffer = 34963,
};
const char* ToString(BufferViewTarget target) {
  switch (target) {
    case BufferViewTarget::kArrayBuffer: return "Array Buffer";
    case BufferViewTarget::kElementArrayBuffer: return "Element Array Buffer";
  }

  NOT_REACHED();
  return "<unknown>";
}

enum class ComponentType : int {
  kInt8 = 5120,
  kUint8 = 5121,
  kInt16 = 5122,
  kUint16 = 5123,
  kInt32 = 5124,
  kUInt32 = 5125,
  kFloat = 5126,
  kDouble = 5130,
};

const char* ToString(ComponentType type) {
  switch (type) {
    case ComponentType::kInt8: return "Int8";
    case ComponentType::kUint8: return "Uint8";
    case ComponentType::kInt16: return "Int16";
    case ComponentType::kUint16: return "Uint16";
    case ComponentType::kInt32: return "Int32";
    case ComponentType::kUInt32: return "UInt32";
    case ComponentType::kFloat: return "Float";
    case ComponentType::kDouble: return "Double";
    default: break;
  }

  NOT_REACHED();
  return "<unknown>";
}

uint32_t ToSize(ComponentType component_type) {
  switch (component_type) {
    case ComponentType::kInt8: return 1;
    case ComponentType::kUint8: return 1;
    case ComponentType::kInt16: return 2;
    case ComponentType::kUint16: return 2;
    case ComponentType::kInt32: return 4;
    case ComponentType::kUInt32: return 4;
    case ComponentType::kFloat: return 4;
    case ComponentType::kDouble: return 8;
  }

  NOT_REACHED();
  return 0;
}

enum class AccessorKind : int {
  kPosition,
  kNormal,
  kTangent,
  kTexcoord0,
  kTexcoord1,
  kColor,
  kJoints,
  kWeights,

  kLast,
};

// clang-format off
AccessorKind AccessorKindFromString(const std::string& kind) {
  if (kind == "POSITION")    { return AccessorKind::kPosition; }
  if (kind == "NORMAL")      { return AccessorKind::kNormal; }
  if (kind == "TANGENT")     { return AccessorKind::kTangent; }
  if (kind == "TEXCOORD_0")  { return AccessorKind::kTexcoord0; }
  if (kind == "TEXCOORD_1")  { return AccessorKind::kTexcoord1; }
  if (kind == "COLOR_0")     { return AccessorKind::kColor; }
  if (kind == "JOINTS_0")    { return AccessorKind::kJoints; }
  if (kind == "WEIGHTS_0")   { return AccessorKind::kWeights; }

  return AccessorKind::kLast;
}
// clang-format on

enum class AccessorType : int {
  kVec2 = 2,
  kVec3 = 3,
  kVec4 = 4,
  kMat2 = 32 + 2,
  kMat3 = 32 + 3,
  kMat4 = 32 + 4,
  kScalar = 64 + 1,
  kVector = 64 + 4,
  kMatrix = 64 + 16,
};
const char* ToString(AccessorType type) {
  switch (type) {
    case AccessorType::kVec2: return "Vec2";
    case AccessorType::kVec3: return "Vec3";
    case AccessorType::kVec4: return "Vec4";
    case AccessorType::kMat2: return "Mat2";
    case AccessorType::kMat3: return "Mat3";
    case AccessorType::kMat4: return "Mat4";
    case AccessorType::kScalar: return "Scalar";
    case AccessorType::kVector: return "Vector";
    case AccessorType::kMatrix: return "Matrix";
  }

  NOT_REACHED();
  return "<unknown>";
}

VertexType DetectVertexType(
    const tinygltf::Model& model,
    const tinygltf::Primitive& primitive,
    std::map<VertComponent, const tinygltf::Accessor*>* accessors = nullptr) {
  uint32_t types = 0;
  for (auto& [attr_name, attr_accessor_index] : primitive.attributes) {
    const tinygltf::Accessor& accessor = model.accessors[attr_accessor_index];

    AccessorKind accessor_kind = AccessorKindFromString(attr_name);
    ComponentType comp_type = (ComponentType)accessor.componentType;
    AccessorType accessor_type = (AccessorType)accessor.type;

    VertComponent component = VertComponent::kLast;

    switch (accessor_kind) {
      case AccessorKind::kPosition: component = VertComponent::kPos3d; break;
      case AccessorKind::kNormal: component = VertComponent::kNormal; break;
      case AccessorKind::kTangent: component = VertComponent::kTangent; break;
      case AccessorKind::kTexcoord0: {
        switch (comp_type) {
          case ComponentType::kUint8: component = VertComponent::kUV0_byte; break;
          case ComponentType::kUint16: component = VertComponent::kUV0_short; break;
          case ComponentType::kFloat: component = VertComponent::kUV0_float; break;
          default: NOT_REACHED();
        }
        break;
      }
      case AccessorKind::kTexcoord1: {
        switch (comp_type) {
          case ComponentType::kUint8: component = VertComponent::kUV1_byte; break;
          case ComponentType::kUint16: component = VertComponent::kUV1_short; break;
          case ComponentType::kFloat: component = VertComponent::kUV1_float; break;
          default: NOT_REACHED();
        }
        break;
      }
      case AccessorKind::kColor: {
        if (accessor_type == AccessorType::kVec3) {
          switch (comp_type) {
            case ComponentType::kUint8: component = VertComponent::kColorRGB_byte; break;
            case ComponentType::kUint16: component = VertComponent::kColorRGB_short; break;
            case ComponentType::kFloat: component = VertComponent::kColorRGB_float; break;
            default: NOT_REACHED();
          }
        } else if (accessor_type == AccessorType::kVec4) {
          switch (comp_type) {
            case ComponentType::kUint8: component = VertComponent::kColorRGBA_byte; break;
            case ComponentType::kUint16: component = VertComponent::kColorRGBA_short; break;
            case ComponentType::kFloat: component = VertComponent::kColorRGBA_float; break;
            default: NOT_REACHED();
          }
        }
        break;
      }
      case AccessorKind::kJoints: {
        switch (comp_type) {
          case ComponentType::kUint8: component = VertComponent::kJoints_byte; break;
          case ComponentType::kUint16: component = VertComponent::kJoints_short; break;
          default: NOT_REACHED();
        }
        break;
      }
      case AccessorKind::kWeights: {
        switch (comp_type) {
          case ComponentType::kUint8: component = VertComponent::kWeights_byte; break;
          case ComponentType::kUint16: component = VertComponent::kWeights_short; break;
          case ComponentType::kFloat: component = VertComponent::kWeights_float; break;
          default: NOT_REACHED();
        }
        break;
      }
      case AccessorKind::kLast: NOT_REACHED(); break;
    }

    ASSERT(component != VertComponent::kLast);
    types |= (uint32_t)component;

    if (accessors)
      (*accessors)[component] = &accessor;
  }

  return ToVertexType(types);
}

struct VerticesExtraction {
  std::vector<uint8_t> data;
  Vec3 min;
  Vec3 max;
};
VerticesExtraction ExtractVertices(const tinygltf::Model& model,
                                   const tinygltf::Primitive& primitive,
                                   VertexType vertex_type = VertexType::kLast) {
  std::map<VertComponent, const tinygltf::Accessor*> accessors;
  vertex_type = DetectVertexType(model, primitive, &accessors);
  uint32_t vertex_size = ToSize(vertex_type);

  ASSERT(vertex_type != VertexType::kLast);
  ASSERT(!accessors.empty());

  // Go over each vertex component and add it to a buffer.
  auto accessor_it = accessors.begin();
  uint32_t vertices_size = accessor_it->second->count * vertex_size;

  std::vector<uint8_t> vertices;
  vertices.resize(vertices_size);  // We're going to overwrite the contents.

  constexpr float kMaxBound = 10000;
  Vec3 min_pos = {kMaxBound, kMaxBound, kMaxBound};
  Vec3 max_pos = {-kMaxBound, -kMaxBound, -kMaxBound};

  // Accessors are already sorted to where they are in the buffer.
  uint32_t component_offset = 0;
  for (; accessor_it != accessors.end(); accessor_it++) {
    const VertComponent& vert_component = accessor_it->first;
    uint32_t component_size = ToSize(vert_component);

    const tinygltf::Accessor* accessor = accessor_it->second;

    const tinygltf::BufferView& buffer_view = model.bufferViews[accessor->bufferView];
    const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

    uint8_t* vertices_ptr = vertices.data() + component_offset;
    uint8_t* vertices_end = vertices.data() + vertices_size;
    const uint8_t* buffer_ptr =
        (const uint8_t*)buffer.data.data() + buffer_view.byteOffset + accessor->byteOffset;
    const uint8_t* buffer_end = (const uint8_t*)buffer.data.data() + buffer.data.size();

    // Copy over the data to the buffer.
    for (size_t i = 0; i < accessor->count; i++) {
      ASSERT(vertices_ptr < vertices_end);
      ASSERT(buffer_ptr < buffer_end);

      if (vert_component == VertComponent::kPos3d) {
        const Vec3& pos = *(const Vec3*)buffer_ptr;
        // clang-format off
        if (min_pos.x > pos.x) { min_pos.x = pos.x; }
        if (min_pos.y > pos.y) { min_pos.y = pos.y; }
        if (min_pos.z > pos.z) { min_pos.z = pos.z; }

        if (max_pos.x < pos.x) { max_pos.x = pos.x; }
        if (max_pos.y < pos.y) { max_pos.y = pos.y; }
        if (max_pos.z < pos.z) { max_pos.z = pos.z; }
        // clang-format on
      }

      // Copy over the component value.
      uint8_t* write_ptr = vertices_ptr;
      for (size_t j = 0; j < component_size; j++) {
        *write_ptr++ = *buffer_ptr++;
      }

      // Advance by the stride.
      buffer_ptr += buffer_view.byteStride;
      vertices_ptr += vertex_size;
    }

    component_offset += component_size;
  }

  VerticesExtraction extraction = {};
  extraction.data = std::move(vertices);
  extraction.min = min_pos;
  extraction.max = max_pos;
  return extraction;
}

template <typename T>
std::vector<uint32_t> ObtainIndices(const uint8_t* data, uint32_t count) {
  std::vector<uint32_t> indices;
  indices.reserve(count);

  const T* ptr = (const T*)data;
  for (uint32_t i = 0; i < count; i++) {
    indices.push_back(*ptr++);
  }

  return indices;
}

std::vector<uint32_t> ExtractIndices(const tinygltf::Model& model,
                                     const tinygltf::Primitive& primitive) {
  const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
  uint32_t index_count = accessor.count;

  ComponentType component_type = (ComponentType)accessor.componentType;
  ASSERT(component_type == ComponentType::kUint16 || component_type == ComponentType::kUInt32);

  const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
  const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

  const uint8_t* data = (const uint8_t*)buffer.data.data();
  data += buffer_view.byteOffset;

  return component_type == ComponentType::kUint16 ? ObtainIndices<uint16_t>(data, index_count)
                                                  : ObtainIndices<uint32_t>(data, index_count);
}

Vec3 NodeToVec3(const double* d) { return {(float)d[0], (float)d[1], (float)d[2]}; }

NO_DISCARD Transform ProcessNodeTransform(const tinygltf::Node& node) {
  Transform transform = {};
  if (!node.matrix.empty())
    return TransformMatrixToTransform(*(Mat4*)node.matrix.data());

  if (!node.translation.empty())
    transform.position = NodeToVec3(node.translation.data());

  if (!node.rotation.empty()) {
    Quaternion q = {};
    q.x = node.rotation[0];
    q.y = node.rotation[1];
    q.z = node.rotation[2];
    q.w = node.rotation[3];
    transform.rotation = ToEuler(q);
  }

  if (!node.scale.empty())
    transform.scale = NodeToVec3(node.scale.data());
  return transform;
}

Texture* LoadTexture(const tinygltf::Model& model,
                     const tinygltf::Material& material,
                     ProcessingContext* context) {
  int texture_index = material.pbrMetallicRoughness.baseColorTexture.index;
  if (texture_index == -1)
    return nullptr;

  // Load the related texture.
  Texture* texture_ptr = nullptr;
  auto texture_it = context->textures.find(texture_index);
  if (texture_it != context->textures.end()) {
    texture_ptr = texture_it->second.get();
  } else {
    const tinygltf::Texture& base_texture =
        model.textures[material.pbrMetallicRoughness.baseColorTexture.index];

    const tinygltf::Image& base_image = model.images[base_texture.source];

    auto rothko_texture = std::make_unique<Texture>();
    rothko_texture->name = base_image.name;
    rothko_texture->size = {base_image.width, base_image.height};
    // TODO(Cristian): Do tinygltf -> rothko modes translation instead of hardcoding.
    //                 This is obtained from |base_texture.sampler|.
    rothko_texture->name = model.images[base_texture.source].uri;
    rothko_texture->type = TextureType::kRGBA;
    rothko_texture->wrap_mode_u = TextureWrapMode::kRepeat;
    rothko_texture->wrap_mode_v = TextureWrapMode::kRepeat;
    rothko_texture->min_filter = TextureFilterMode::kLinear;
    rothko_texture->mag_filter = TextureFilterMode::kLinear;

    ASSERT(DataSize(*rothko_texture) == base_image.image.size());
    rothko_texture->data = std::make_unique<uint8_t[]>(base_image.image.size());
    memcpy(rothko_texture->data.get(), base_image.image.data(), base_image.image.size());

    texture_ptr = rothko_texture.get();
    context->textures[base_texture.source] = std::move(rothko_texture);
  }

  return texture_ptr;
}

Material* HandleMaterial(const tinygltf::Model& model,
                         const tinygltf::Primitive& primitive,
                         ProcessingContext* context) {
  auto material_it = context->materials.find(primitive.material);
  if (material_it != context->materials.end())
    return material_it->second.get();

  const tinygltf::Material& material = model.materials[primitive.material];

  auto rothko_material = std::make_unique<Material>();
  rothko_material->base_texture = LoadTexture(model, material, context);
  ASSERT(material.pbrMetallicRoughness.baseColorFactor.size() == 4u);
  rothko_material->base_color.r = material.pbrMetallicRoughness.baseColorFactor[0];
  rothko_material->base_color.g = material.pbrMetallicRoughness.baseColorFactor[1];
  rothko_material->base_color.b = material.pbrMetallicRoughness.baseColorFactor[2];
  rothko_material->base_color.a = material.pbrMetallicRoughness.baseColorFactor[3];

  Material* material_ptr = rothko_material.get();
  context->materials[primitive.material] = std::move(rothko_material);
  return material_ptr;
}

bool ProcessNode(const tinygltf::Model& model,
                 const tinygltf::Node& node,
                 const NodeContext& parent,
                 ProcessingContext* context,
                 NodeContext* node_context) {
  ModelNode& model_node = context->model.nodes.emplace_back();
  SceneNode* current_node = AddNode(context->scene_graph.get(), parent.scene_node);
  current_node->transform = ProcessNodeTransform(node);

  node_context->scene_node = current_node;
  node_context->index = context->model.nodes.size() - 1;
  node_context->parent_index = parent.index;
  context->scene_nodes.push_back(*node_context);

  // This is just a transform containing node.
  if (node.mesh == -1)
    return true;

  // Check if we loaded the mesh.
  if (context->processed_node_meshes.count(node.mesh)) {
    NOT_REACHED_MSG("Mesh reuse not supported yet.");
    return node_context;
  }

  const tinygltf::Mesh& mesh = model.meshes[node.mesh];

  // Process the primitives.
  for (uint32_t primitive_i = 0; primitive_i < mesh.primitives.size(); primitive_i++) {
    const tinygltf::Primitive& primitive = mesh.primitives[primitive_i];
    VertexType vertex_type = DetectVertexType(model, primitive);

    auto [vertices, min, max] = ExtractVertices(model, primitive);
    uint32_t vertex_count = vertices.size() / ToSize(vertex_type);

    // TODO(Cristian): Right now we handle only k3dNormalUV;
    if (vertex_type == VertexType::k3dNormalTangentUV) {
      // We transform the vertices into the supported vertex format.
      std::vector<uint8_t> new_vertices;
      new_vertices.reserve(sizeof(Vertex3dNormalUV) * vertex_count);

      const auto* vertex_ptr = (const Vertex3dNormalTangentUV*)vertices.data();
      for (uint32_t i = 0; i < vertex_count; i++) {
        Vertex3dNormalUV vertex = {};
        vertex.pos = vertex_ptr->pos;
        vertex.normal = vertex_ptr->normal;
        vertex.uv = vertex_ptr->uv;

        uint8_t* ptr = (uint8_t*)&vertex;
        for (uint32_t j = 0; j < sizeof(vertex); j++) {
          new_vertices.emplace_back(*ptr++);
        }

        /* new_vertices.emplace_back(std::move(vertex)); */
        vertex_ptr++;
      }
      vertices = std::move(new_vertices);
      vertex_type = VertexType::k3dNormalUV;
    } else if (vertex_type != VertexType::k3dNormalUV) {
      ERROR(App, "Unsupported vertex type: %s", ToString(vertex_type));
      return false;
    }

    std::vector<uint32_t> indices = ExtractIndices(model, primitive);

    // Create the mesh.
    auto rothko_mesh = std::make_unique<Mesh>();
    rothko_mesh->name = StringPrintf("%s-%u", mesh.name.c_str(), primitive_i);
    rothko_mesh->vertex_type = vertex_type;
    rothko_mesh->vertices = std::move(vertices);
    rothko_mesh->vertex_count = vertex_count;
    rothko_mesh->indices = std::move(indices);
    const Mesh* mesh_ptr = rothko_mesh.get();
    context->meshes.push_back(std::move(rothko_mesh));

    /* LOG(App, */
    /*     "Mesh for %s: %s -> Vertices: %u (Min: %s, Max: %s), Indices: %zu", */
    /*     node.name.c_str(), */
    /*     mesh_ptr->name.c_str(), */
    /*     mesh_ptr->vertex_count, */
    /*     ToString(min).c_str(), */
    /*     ToString(max).c_str(), */
    /*     mesh_ptr->indices.size()); */

    // Material.
    model_node.primitives[primitive_i].bounds.min = min;
    model_node.primitives[primitive_i].bounds.max = max;
    model_node.primitives[primitive_i].mesh = mesh_ptr;
    model_node.primitives[primitive_i].material = HandleMaterial(model, primitive, context);
  }

  context->processed_node_meshes.insert(node.mesh);

  return true;
}

bool ProcessNodes(const tinygltf::Model& model, const tinygltf::Node& node,
                  const NodeContext& parent_node, ProcessingContext* context) {
  NodeContext current_node = {};
  if (!ProcessNode(model, node, parent_node, context, &current_node))
    return false;

  for (int node_index : node.children) {
    if (!ProcessNodes(model, model.nodes[node_index], current_node, context))
      return false;
  }

  return true;
}

bool ProcessModel(const tinygltf::Model& model, const tinygltf::Scene& scene, Model* model_out) {
  ProcessingContext context = {};
  context.scene_graph = std::make_unique<SceneGraph>();

  for (int node_index : scene.nodes) {
    if (!ProcessNodes(model, model.nodes[node_index], {}, &context))
      return false;
  }

  // Fill in the context into the model.
  *model_out = std::move(context.model);

  model_out->meshes = std::move(context.meshes);

  model_out->textures.reserve(context.textures.size());
  for (auto& [id, texture] : context.textures) {
    model_out->textures.push_back(std::move(texture));
  }

  model_out->materials.reserve(context.materials.size());
  for (auto& [id, material] : context.materials) {
    model_out->materials.push_back(std::move(material));
  }

  // Once we have processed all the nodes, we need to correctly set the internal scene graph.
  Update(context.scene_graph.get());
  for (uint32_t i = 0; i < context.scene_nodes.size(); i++) {
    const NodeContext& scene_node = context.scene_nodes[i];
    model_out->nodes[i].transform = scene_node.scene_node->transform;
  }

  return true;
}

}  // namespace

bool LoadModel(const std::string& path, Model* model_out) {
    std::string err, warn;
    tinygltf::TinyGLTF gltf_loader;
    tinygltf::Model gltf_model = {};
    if (!gltf_loader.LoadASCIIFromFile(&gltf_model, &err, &warn, path)) {
      ERROR(Model, "Could not load model %s: %s.", path.c_str(), err.c_str());
      return false;
    }

    if (!warn.empty())
      WARNING(Model, "Loading model %s: %s.", path.c_str(), warn.c_str());

    // TODO(donosoc): Load mode than the default scene.
    if (gltf_model.scenes.size() > 1u) {
      WARNING(Model, "Model %s: More than one scene defined. Only default scene is supported.",
              path.c_str());
    }

    auto& gltf_scene = gltf_model.scenes[gltf_model.defaultScene];
    if (!ProcessModel(gltf_model, gltf_scene, model_out))
      return false;

    model_out->name = path;
    return true;
}

}  // namespace gltf
}  // namespace rothko
