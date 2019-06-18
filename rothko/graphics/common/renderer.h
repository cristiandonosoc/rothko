// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "rothko/utils/macros.h"
#include "rothko/graphics/common/renderer_backend.h"

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
  bool frame_started = false;
};

bool Valid(Renderer* r);

struct InitRendererConfig {
  RendererType type = RendererType::kLast;  // Required.
  Window* window = nullptr;                 // Required. Must outlive renderer.
};
bool InitRenderer(Renderer*, InitRendererConfig*);

void StartFrame(Renderer*);
void EndFrame(Renderer*);

// Meshes.
bool RendererStageMesh(Renderer*, Mesh*);
void RendererUnstageMesh(Renderer*, Mesh*);

// Shaders.
bool RendererParseShader(Renderer*,
                         const std::string& vert_path,
                         const std::string& frag_path,
                         Shader* out);
void RendererUnstageShader(Renderer*, Shader*);

// Textures.
bool RendererStageTexture(Renderer*, Texture*);
void RendererUnstageTexture(Renderer*, Texture*);

}  // namespace rothko
