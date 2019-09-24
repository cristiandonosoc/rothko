// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include <third_party/stb/stb_image.h>
#include <third_party/stb/stb_image_write.h>

#define TINYGLTF_NO_INCLUDE_JSON
#include <third_party/tiny_gltf/tiny_gltf_internal.h>

#undef TINYGLTF_NO_INCLUDE_STB_IMAGE
#undef TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#undef TINYGLTF_NO_INCLUDE_JSON
#undef TINYGLTF_USE_CPP14
