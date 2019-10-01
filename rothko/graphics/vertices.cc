// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/vertices.h"

#include "rothko/logging/logging.h"

namespace rothko {

const char* ToString(VertComponent vert_component) {
  switch (vert_component) {
    case VertComponent::kPos2d: return "Pos2d";
    case VertComponent::kPos3d: return "Pos3d";
    case VertComponent::kNormal: return "Normal";
    case VertComponent::kTangent: return "Tangent";
    case VertComponent::kUV0_byte: return "UV0_byte";
    case VertComponent::kUV0_short: return "UV0_short";
    case VertComponent::kUV0_float: return "UV0_float";
    case VertComponent::kUV1_byte: return "UV1_byte";
    case VertComponent::kUV1_short: return "UV1_short";
    case VertComponent::kUV1_float: return "UV1_float";
    case VertComponent::kColorRGB_byte: return "ColorRGB_byte";
    case VertComponent::kColorRGB_short: return "ColorRGB_short";
    case VertComponent::kColorRGB_float: return "ColorRGB_float";
    case VertComponent::kColorRGBA_byte: return "ColorRGBA_byte";
    case VertComponent::kColorRGBA_short: return "ColorRGBA_short";
    case VertComponent::kColorRGBA_float: return "ColorRGBA_float";
    case VertComponent::kJoints_byte: return "Joints_byte";
    case VertComponent::kJoints_short: return "Joints_short";
    case VertComponent::kWeights_byte: return "Weights_byte";
    case VertComponent::kWeights_short: return "Weights_short";
    case VertComponent::kWeights_float: return "Weights_float";
    case VertComponent::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

uint32_t ToSize(VertComponent vert_component) {
  switch (vert_component) {
    case VertComponent::kPos2d: return sizeof(Vec2);
    case VertComponent::kPos3d: return sizeof(Vec3);
    case VertComponent::kNormal: return sizeof(Vec3);
    case VertComponent::kTangent: return sizeof(Vec4);
    case VertComponent::kUV0_byte: return sizeof(uint16_t);
    case VertComponent::kUV0_short: return sizeof(uint32_t);
    case VertComponent::kUV0_float: return sizeof(Vec2);
    case VertComponent::kUV1_byte: return sizeof(uint16_t);
    case VertComponent::kUV1_short: return sizeof(uint32_t);
    case VertComponent::kUV1_float: return sizeof(Vec2);
    case VertComponent::kColorRGB_byte: return sizeof(uint8_t[3]);
    case VertComponent::kColorRGB_short: return sizeof(uint16_t[3]);
    case VertComponent::kColorRGB_float: return sizeof(Vec3);
    case VertComponent::kColorRGBA_byte: return sizeof(uint32_t);
    case VertComponent::kColorRGBA_short: return sizeof(uint64_t);
    case VertComponent::kColorRGBA_float: return sizeof(Vec4);
    case VertComponent::kJoints_byte: return sizeof(uint32_t);
    case VertComponent::kJoints_short: return sizeof(uint64_t);
    case VertComponent::kWeights_byte: return sizeof(uint32_t);
    case VertComponent::kWeights_short: return sizeof(uint64_t);
    case VertComponent::kWeights_float: return sizeof(Vec4);
    case VertComponent::kLast: return 0;
  }

  NOT_REACHED();
  return 0;
}

const char* ToString(VertexType type) {
  switch (type) {
    case VertexType::k2dUVColor: return "2d UV Color";
    case VertexType::k3dColor: return "3d Color";
    case VertexType::k3dNormalUV: return "3d Normal UV";
    case VertexType::k3dNormalTangentUV: return "3d Normal Tangent UV";
    case VertexType::k3dUV: return "3d UV";
    case VertexType::k3dUVColor: return "3D UV Color";
    case VertexType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

uint32_t ToSize(VertexType type) {
  switch (type) {
    case VertexType::k2dUVColor: return sizeof(Vertex2dUVColor);
    case VertexType::k3dColor: return sizeof(Vertex3dColor);
    case VertexType::k3dNormalUV: return sizeof(Vertex3dNormalUV);
    case VertexType::k3dNormalTangentUV: return sizeof(Vertex3dNormalTangentUV);
    case VertexType::k3dUV: return sizeof(Vertex3dUV);
    case VertexType::k3dUVColor: return sizeof(Vertex3dUVColor);
    case VertexType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}

VertexType ToVertexType(uint32_t t) {
  VertexType type = (VertexType)t;

  switch (type) {
    case VertexType::k2dUVColor:
    case VertexType::k3dColor:
    case VertexType::k3dNormalUV:
    case VertexType::k3dUV:
    case VertexType::k3dUVColor:
    case VertexType::k3dNormalTangentUV:
    case VertexType::kLast:
      return type;
  }


  WARNING(Graphics, "Unrecognized vertex type: 0x%x", t);
  return VertexType::kLast;
}

}  // namespace rothko
