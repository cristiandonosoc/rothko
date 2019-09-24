// This is glue for third party tinyGLTF code.
//
// Author: 2019, Cristián Donoso.
// Creative Commons.

#include "tiny_gltf.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14

#define TINYGLTF_NO_INCLUDE_JSON
#include <third_party/json/json.hpp>

#include "tiny_gltf_internal.h"
