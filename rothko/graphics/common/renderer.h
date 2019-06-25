// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "rothko/containers/vector.h"
#include "rothko/graphics/common/render_commands.h"
#include "rothko/graphics/common/renderer_backend.h"
#include "rothko/utils/macros.h"

// Renderer
// =============================================================================
//
// This is the abstraction Rothko provides for handling different graphics APIs.
// From the point of view of the code, this is all they need to know.
//
// Underneath, all the graphics code are delegated to a |RendererBackend|, which
// implements them in term of a particular API (OpenGL, Vulkan, etc.).
//
// The way the code gets an instace of a particular backend is by using factory
// functions. Rothko will maintain a map of functions it can use to create an
// instance of a particular backend. Each backend must suscribe their factory
// function in order to work. See the definitions of the functions below.
//
// See also rothko/graphics/common/renderer_backend.h for more details.

namespace rothko {

struct Mesh;
struct Shader;
struct Texture;
struct Window;

enum class RendererType {
  kOpenGL,
  kLast,
};
const char* ToString(RendererType);

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular RendererBackend.
using RendererBackendFactoryFunction = std::unique_ptr<RendererBackend> (*)();
void SuscribeRendererBackendFactory(RendererType,
                                    RendererBackendFactoryFunction);

// Renderer --------------------------------------------------------------------

struct Renderer {
  RAII_CONSTRUCTORS(Renderer);

  Window* window = nullptr;   // Not owning. must outlive.

  RendererType type = RendererType::kLast;
  std::unique_ptr<RendererBackend> backend = nullptr;
};

bool Valid(Renderer* r);

struct InitRendererConfig {
  RendererType type = RendererType::kLast;  // Required.
  Window* window = nullptr;                 // Required. Must outlive renderer.
};
bool InitRenderer(Renderer*, InitRendererConfig*);

// Meshes.
bool RendererStageMesh(Renderer*, Mesh*);
void RendererUnstageMesh(Renderer*, Mesh*);

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
bool RendererStageTexture(const StageTextureConfig&, Renderer*, Texture*);
void RendererUnstageTexture(Renderer*, Texture*);

void StartFrame(Renderer*);
void RendererExecuteCommands(const PerFrameVector<RenderCommand>&, Renderer*);
void EndFrame(Renderer*);


}  // namespace rothko
