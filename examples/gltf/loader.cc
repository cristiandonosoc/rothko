// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "loader.h"

#include <rothko/graphics/graphics.h>
#include <rothko/graphics/vertices.h>
#include <rothko/logging/logging.h>
#include <rothko/utils/strings.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include <sstream>

namespace rothko {
namespace gltf {

namespace {

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
  kInt8   = 5120,
  kUint8  = 5121,
  kInt16  = 5122,
  kUint16 = 5123,
  kInt32  = 5124,
  kUInt32 = 5125,
  kFloat  = 5126,
  kDouble = 5130,
};
uint32_t ToSize(ComponentType component_type) {
  switch (component_type) {
    case ComponentType::kInt8:    return 1;
    case ComponentType::kUint8:   return 1;
    case ComponentType::kInt16:   return 2;
    case ComponentType::kUint16:  return 2;
    case ComponentType::kInt32:   return 4;
    case ComponentType::kUInt32:  return 4;
    case ComponentType::kFloat:   return 4;
    case ComponentType::kDouble:  return 8;
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

VertexType
DetectVertexType(const tinygltf::Model& model,
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
      case AccessorKind::kNormal:   component = VertComponent::kNormal; break;
      case AccessorKind::kTangent:  component = VertComponent::kTangent; break;
      case AccessorKind::kTexcoord0: {
        switch (comp_type) {
          case ComponentType::kUint8:   component = VertComponent::kUV0_byte; break;
          case ComponentType::kUint16:  component = VertComponent::kUV0_short; break;
          case ComponentType::kFloat:   component = VertComponent::kUV0_float; break;
          default: NOT_REACHED();
        }
        break;
      }
      case AccessorKind::kTexcoord1: {
        switch (comp_type) {
          case ComponentType::kUint8:   component = VertComponent::kUV1_byte; break;
          case ComponentType::kUint16:  component = VertComponent::kUV1_short; break;
          case ComponentType::kFloat:   component = VertComponent::kUV1_float; break;
          default: NOT_REACHED();
        }
        break;
      }
      case AccessorKind::kColor: {
        if (accessor_type == AccessorType::kVec3) {
          switch (comp_type) {
            case ComponentType::kUint8:   component = VertComponent::kColorRGB_byte; break;
            case ComponentType::kUint16:  component = VertComponent::kColorRGB_short; break;
            case ComponentType::kFloat:   component = VertComponent::kColorRGB_float; break;
            default: NOT_REACHED();
          }
        } else if (accessor_type == AccessorType::kVec4) {
          switch (comp_type) {
            case ComponentType::kUint8:   component = VertComponent::kColorRGBA_byte; break;
            case ComponentType::kUint16:  component = VertComponent::kColorRGBA_short; break;
            case ComponentType::kFloat:   component = VertComponent::kColorRGBA_float; break;
            default: NOT_REACHED();
          }
        }
        break;
      }
      case AccessorKind::kJoints: {
        switch (comp_type) {
          case ComponentType::kUint8:   component = VertComponent::kJoints_byte; break;
          case ComponentType::kUint16:  component = VertComponent::kJoints_short; break;
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

std::vector<uint8_t> ExtractVertices(const tinygltf::Model& model,
                                     const tinygltf::Primitive& primitive,
                                     VertexType vertex_type = VertexType::kLast) {
  std::map<VertComponent, const tinygltf::Accessor*> accessors;
  vertex_type = DetectVertexType(model, primitive, &accessors);

  ASSERT(vertex_type != VertexType::kLast);
  ASSERT(!accessors.empty());

  // Go over each vertex component and add it to a buffer.
  auto accessor_it = accessors.begin();
  uint64_t vertices_size = accessor_it->second->count * ToSize(vertex_type);

  std::vector<uint8_t> vertices;
  vertices.resize(vertices_size);   // We're going to overwrite the contents.

  // Accessors are already sorted to where they are in the buffer.
  uint32_t component_offset = 0;
  for(; accessor_it != accessors.end(); accessor_it++) {
    const VertComponent& vert_component = accessor_it->first;
    uint32_t component_size = ToSize(vert_component);

    auto& accessor = accessor_it->second;
    const tinygltf::BufferView& buffer_view = model.bufferViews[accessor->bufferView];
    const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

    uint8_t* vertices_ptr = vertices.data() + component_offset;
    uint8_t* vertices_end = vertices.data() + vertices_size;
    const uint8_t* buffer_ptr = (const uint8_t*)buffer.data.data() + accessor->byteOffset;
    const uint8_t* buffer_end = (const uint8_t*)buffer.data.data() + buffer.data.size();

    // Copy over the data to the buffer.
    for (size_t i = 0; i < accessor->count; i++) {
      ASSERT(vertices_ptr < vertices_end);
      ASSERT(buffer_ptr < buffer_end);

      // Copy over the component value.
      for (size_t j = 0; j < component_size; j++) {
        *vertices_ptr++ = *buffer_ptr++;
      }

      // Advance by the stride.
      buffer_ptr += buffer_view.byteStride;
    }

    component_offset += component_size;
  }

  return vertices;
}

std::vector<uint8_t> ExtractIndices(const tinygltf::Model& model,
                                    const tinygltf::Primitive& primitive,
                                    ComponentType* out = nullptr) {
  const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
  ComponentType component_type = (ComponentType)accessor.componentType;
  uint32_t index_size = ToSize(component_type);
  uint64_t indices_size = index_size * accessor.count;

  const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
  const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

  std::vector<uint8_t> indices;
  indices.resize(indices_size);   // We're going to overwrite the contents.

  uint8_t* indices_ptr = indices.data();
  uint8_t* indices_end = indices_ptr + indices_size;
  const uint8_t* buffer_ptr = (const uint8_t*)buffer.data.data() + accessor.byteOffset;
  const uint8_t* buffer_end = (const uint8_t*)buffer.data.data() + buffer.data.size();

  for (size_t i = 0; i < accessor.count; i++) {
    ASSERT(indices_ptr < indices_end);
    ASSERT(buffer_ptr < buffer_end);

    for (size_t j = 0; j < index_size; j++) {
      *indices_ptr++ = *buffer_ptr++;
    }
  }

  if (out)
    *out = component_type;

  return indices;
}

void ProcessNode(const tinygltf::Model& model, const tinygltf::Node& node, Scene* out_scene) {
  LOG(App, "Processing Node %s", node.name.c_str());
  if (node.mesh == -1) {
    WARNING(App, "Node %s has no mesh.", node.name.c_str());
    return;
  }

  const tinygltf::Mesh& mesh = model.meshes[node.mesh];
  LOG(App, "Processing mesh %s", mesh.name.c_str());

  std::stringstream ss;
  ss << "Primitives: " << std::endl;

  // Process the primitives.
  for (uint32_t primitive_i = 0; primitive_i < mesh.primitives.size(); primitive_i++) {
    const tinygltf::Primitive& primitive = mesh.primitives[primitive_i];
    ss << "  Primitive " << primitive_i << std::endl;

    VertexType vertex_type = DetectVertexType(model, primitive);
    ss << "    Vertex Type: " << ToString(vertex_type) << ", Size: " << ToSize(vertex_type)
       << std::endl;

    ss << "    Indices -> Accessor " << primitive.indices << std::endl;

    for (auto& [attr_name, attr_accessor_index] : primitive.attributes) {
      ss << "    " << "Attribute " << attr_name << " -> Accessor " << attr_accessor_index;

      const tinygltf::Accessor& accessor = model.accessors[attr_accessor_index];
      if (!accessor.name.empty())
        ss << " (" << accessor.name << ")";
      ss << std::endl;
      ss << "      Type: " << ToString((AccessorType)accessor.type) << std::endl;

      const tinygltf::BufferView bv = model.bufferViews[accessor.bufferView];

      ss << "      Buffer view " << bv.name << std::endl;
      ss << "        offset: " << bv.byteOffset << std::endl;
      ss << "        length: " << bv.byteLength << std::endl;
      ss << "        stride: " << bv.byteStride << std::endl;
      ss << "        target: " << bv.target << " (" << ToString((BufferViewTarget)bv.target) << ")"
         << std::endl;
    }

    std::vector<uint8_t> vertices = ExtractVertices(model, primitive);
    uint32_t vertex_count = vertices.size() / ToSize(vertex_type);

    ComponentType component_type;
    std::vector<uint8_t> indices = ExtractIndices(model, primitive, &component_type);
    uint32_t index_count = indices.size() / ToSize(component_type);

    ss << "    VERTICES: " << vertices.size() << " bytes (" << vertex_count << " vertices)."
       << std::endl;
    ss << "    INDICES: " << indices.size() << " bytes (" << index_count << " indices)."
       << std::endl;

    (void)out_scene;

#ifdef COMPLETE_ME
    // Create the mesh.
    auto rothko_mesh = std::make_unique<Mesh>();
    rothko_mesh->name = StringPrintf("%s-%u", mesh.name.c_str(), primitive_i);
    rothko_mesh->vertex_type = vertex_type;
    rothko_mesh->vertices = std::move(vertices);
    rothko_mesh->vertex_count = vertex_count;
    rothko_mesh->indices = std::move(indices);
    rothko_mesh->index_count = index_count;

    const tinygltf::Material& material = model.materials[primitive.material];

    const tinygltf::Texture& base_texture =
        model.textures[material.pbrMetallicRoughness.baseColorTexture.index];

    const tinygltf::Sampler& base_sampler = model.samplers[base_texture.sampler];
    const tinygltf::Image& base_image = model.images[base_texture.source];

    // TODO(Cristian): Do tinygltf -> rothko modes translation instead of hardcoding.
    auto rothko_texture = std::make_unique<Texture>();
    rothko_texture->type = TextureType::kRGBA;
    rothko_texture->wrap_mode_s = TextureWrapMode::kRepeat;
    rothko_texture->wrap_mode_t = TextureWrapMode::kRepeat;
    rothko_texture->min_filter = TextureFilterMode::kLinear;
    rothko_texture->mag_filter = TextureFilterMode::kLinear;





    auto rothko_material = std::make_unique<Material>();


    rothko_material->base_texture = material.pbrMetallicRoughness.baseColorTexture.

    SceneNode scene_node;
    scene_node.mesh = rothko_mesh.get();



    out_scene->meshes.push_back(std::move(rothko_mesh));

#endif
  }

  LOG(App, "%s", ss.str().c_str());
}



void ProcessNodes(const tinygltf::Model& model, const tinygltf::Node& node, Scene* out_scene) {
  ProcessNode(model, node, out_scene);
  for (int node_index : node.children) {
    ProcessNodes(model, model.nodes[node_index], out_scene);
  }
}

}  // namespace


void ProcessScene(const tinygltf::Model& model, const tinygltf::Scene& scene, Scene* out_scene) {
  for (int node_index : scene.nodes) {
    ProcessNodes(model, model.nodes[node_index], out_scene);
  }
}

}  // namespace gltf
}  // namespace rothko