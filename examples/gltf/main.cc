// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/platform/platform.h>
#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include <sstream>

using namespace rothko;

namespace {

struct Mesh {
  std::vector<uint8_t> buffers;

  std::vector<uint8_t> indices;
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
  kInt8   = 5120,
  kUint8  = 5121,
  kInt16  = 5122,
  kUint16 = 5123,
  kInt32  = 5124,
  kUInt32 = 5125,
  kFloat  = 5126,
  kDouble = 5130,
};

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

inline

VertexType DetectVertexType(const tinygltf::Model& model, const tinygltf::Primitive& primitive) {
  uint32_t types = 0;
  for (auto& [attr_name, attr_accessor_index] : primitive.attributes) {
    const tinygltf::Accessor& accessor = model.accessors[attr_accessor_index];

    AccessorKind accessor_kind = AccessorKindFromString(attr_name);
    ComponentType comp_type = (ComponentType)accessor.componentType;
    AccessorType accessor_type = (AccessorType)accessor.type;

    switch (accessor_kind) {
      case AccessorKind::kPosition: types |= kVertCompPos3d; continue;
      case AccessorKind::kNormal:   types |= kVertCompNormal; continue;
      case AccessorKind::kTangent:  types |= kVertCompTangent; continue;
      case AccessorKind::kTexcoord0: {
        switch (comp_type) {
          case ComponentType::kUint8:   types |= kVertCompUV0_byte; continue;
          case ComponentType::kUint16:  types |= kVertCompUV0_short; continue;
          case ComponentType::kFloat:   types |= kVertCompUV0_float; continue;
          default: NOT_REACHED();
        }
        continue;
      }
      case AccessorKind::kTexcoord1: {
        switch (comp_type) {
          case ComponentType::kUint8:   types |= kVertCompUV1_byte; continue;
          case ComponentType::kUint16:  types |= kVertCompUV1_short; continue;
          case ComponentType::kFloat:   types |= kVertCompUV1_float; continue;
          default: NOT_REACHED();
        }
        continue;
      }
      case AccessorKind::kColor: {
        if (accessor_type == AccessorType::kVec3) {
          switch (comp_type) {
            case ComponentType::kUint8:   types |= kVertCompColorRGB_byte; continue;
            case ComponentType::kUint16:  types |= kVertCompColorRGB_short; continue;
                                          io
            case ComponentType::kFloat:   types |= kVertCompColorRGB_float; continue;
            default: NOT_REACHED();
          }
        } else if (accessor_type == AccessorType::kVec4) {
          switch (comp_type) {
            case ComponentType::kUint8:   types |= kVertCompColorRGBA_byte; continue;
            case ComponentType::kUint16:  types |= kVertCompColorRGBA_short; continue;
            case ComponentType::kFloat:   types |= kVertCompColorRGBA_float; continue;
            default: NOT_REACHED();
          }
        }
        continue;
      }
      case AccessorKind::kJoints: {
        switch (comp_type) {
          case ComponentType::kUint8:   types |= kVertCompJoints_byte; continue;
          case ComponentType::kUint16:  types |= kVertCompJoints_short; continue;
          default: NOT_REACHED();
        }
        continue;
      }
      case AccessorKind::kWeights: {
        switch (comp_type) {
          case ComponentType::kUint8:   types |= kVertCompWeights_byte; continue;
          case ComponentType::kUint16:  types |= kVertCompWeights_short; continue;
          case ComponentType::kFloat:   types |= kVertCompWeights_float; continue;
          default: NOT_REACHED();
        }
        continue;
      }
      case AccessorKind::kLast:
        NOT_REACHED();
        continue;
    }
  }

  return ToVertexType(types);
}

void ProcessNode(const tinygltf::Model& model, const tinygltf::Node& node) {
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
  for (size_t primitive_i = 0; primitive_i < mesh.primitives.size(); primitive_i++) {
    const tinygltf::Primitive& primitive = mesh.primitives[primitive_i];
    ss << "  Primitive " << primitive_i << std::endl;

    VertexType vertex_type = DetectVertexType(model, primitive);
    ss << "    Vertex Type: " << ToString(vertex_type) << std::endl;

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
  }

  LOG(App, "%s", ss.str().c_str());
}

void ProcessNodes(const tinygltf::Model& model, const tinygltf::Node& node) {
  ProcessNode(model, node);
  for (int node_index : node.children) {
    ProcessNodes(model, model.nodes[node_index]);
  }
}

}  // namespace

int main() {
  auto log_handle = InitLoggingSystem(true);
  auto platform = InitializePlatform();
  /* Game game = {}; */
  /* InitWindowConfig window_config = {}; */
  /* window_config.type = WindowType::kSDLOpenGL; */
  /* window_config.resizable = true; */
  /* /1* window_config.fullscreen = true; *1/ */
  /* window_config.screen_size = {1920, 1440}; */
  /* if (!InitGame(&game, &window_config, true)) */
  /*   return 1; */



  std::string path = OpenFileDialog();
  if (path.empty()) {
    ERROR(App, "Could not get model path.");
    return 1;
  }

  std::string err, warn;

  tinygltf::TinyGLTF gltf_loader;
  tinygltf::Model model = {};
  if (!gltf_loader.LoadASCIIFromFile(&model, &err, &warn, path)) {
    ERROR(App, "Could not load model: %s", err.c_str());
    return 1;
  }

  if (!warn.empty()) {
    WARNING(App, "At loading model %s: %s", path.c_str(), warn.c_str());
  }

  LOG(App, "Loaded model!");

  // Go over the scene.
  auto& scene = model.scenes[model.defaultScene];
  LOG(App, "Processing scene %s", scene.name.c_str());
  for (int node_index : scene.nodes) {
    ProcessNodes(model, model.nodes[node_index]);
  }
}
