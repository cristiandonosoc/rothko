// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/vertices.h"

#include "rothko/logging/logging.h"

namespace rothko {

const char* ToString(VertexType type) {
  switch (type) {
    case VertexType::k2dUVColor: return "2d UV Color";
    case VertexType::k3dColor: return "3d Color";
    case VertexType::k3dNormalUV: return "3d Normal UV";
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
    case VertexType::k3dUV: return sizeof(Vertex3dUV);
    case VertexType::k3dUVColor: return sizeof(Vertex3dUVColor);
    case VertexType::kLast: break;
  }

  NOT_REACHED();
  return 0;
}



}  // namespace rothko
