// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "scene.h"

#include <rothko/graphics/graphics.h>
#include <rothko/logging/logging.h>
#include <rothko/models/model.h>
#include <rothko/utils/file.h>

namespace rothko {
namespace gltf {

// Header Titles -----------------------------------------------------------------------------------

const char* Header::kTitle          = "**RTHK**";

const char* MeshesHeader::kTitle    = "*MESHES*";
const char* MeshHeader::kTitle      = "**MESH**";

const char* TexturesHeader::kTitle  = "*TXTRES*";
const char* TextureHeader::kTitle   = "**TXTR**";

// clang-format on

}  // namespace gltf
}  // namespace rothko
