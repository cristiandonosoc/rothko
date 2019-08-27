// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "rothko/containers/vector.h"
#include "rothko/graphics/commands.h"
#include "rothko/math/math.h"
#include "rothko/utils/macros.h"

// Renderer
// =================================================================================================
//
// This is the abstraction Rothko provides for handling different graphics APIs.
// From the point of view of the code, this is all they need to know.
//
// Each backend needs to implements this interface and will be linked in on compile time.

namespace rothko {

struct Mesh;
struct Renderer;
struct Shader;
struct Texture;
struct Window;

// Renderer ----------------------------------------------------------------------------------------

std::unique_ptr<Renderer> InitRenderer();
void ShutdownRenderer();

struct Renderer {
  ~Renderer() {
    ShutdownRenderer();
  }

  // TODO(Cristian): Add statistics about loaded assets?
  const char* renderer_type = nullptr;
};

inline bool Valid(const Renderer* r) { return !!r->renderer_type; }

// Meshes.
bool RendererStageMesh(Renderer*, Mesh*);
void RendererUnstageMesh(Renderer*, Mesh*);

// Re-uploads the contents of the mesh into the already staged buffer. Hence, the mesh needs to be
// already staged.
//
// |vertex_range| represents what section of vertices to upload. X = offset, Y = size.
// |index_range| represents what section of indices to upload. X = offset, Y = size.
//
// For both ranges, empty size means all.
bool RendererUploadMeshRange(Renderer*, Mesh*, Int2 vertex_range = {}, Int2 index_range = {});

// Shaders.
bool RendererStageShader(Renderer*, Shader*);
void RendererUnstageShader(Renderer*, Shader*);

// Textures.
struct StageTextureConfig {
  enum class Wrap {
    kClampToEdge,
    kMirroredRepeat,
    kRepeat,
  };

  enum class Filter {
    kNearest,
    kLinear,
    kNearestMipmapNearest,
    kNearestMipmapLinear,
    kLinearMipmapNearest,
    kLinearMipampLinear,
  };

  bool generate_mipmaps = true;

  Wrap wrap_u = Wrap::kRepeat;
  Wrap wrap_v = Wrap::kRepeat;

  Filter min_filter = Filter::kLinear;
  Filter max_filter = Filter::kLinear;
};
bool RendererStageTexture(Renderer*, Texture*, const StageTextureConfig&);
void RendererUnstageTexture(Renderer*, Texture*);

// |data| being nullptr means upload the data that's currently in the |texture.data.value|.
// |offset| and |range| being zero means upload the whole texture.
void RendererSubTexture(Renderer*, Texture*, void* data = nullptr, Int2 offset = {}, Int2 range = {});

void RendererStartFrame(Renderer*);
void RendererExecuteCommands(Renderer*, const PerFrameVector<RenderCommand>&);
void RendererEndFrame(Renderer*, Window*);

}  // namespace rothko
