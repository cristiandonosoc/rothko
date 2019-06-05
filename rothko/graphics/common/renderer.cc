// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/renderer.h"

#include <map>

#include "rothko/graphics/common/renderer_backend.h"
#include "rothko/utils/logging.h"
#include "rothko/window/window.h"

namespace rothko {

// Backend Suscription ---------------------------------------------------------

namespace {

void Reset(Renderer* renderer) {
  renderer->window = nullptr;
  renderer->backend.reset();
}

using FactoryMap =
    std::map<RendererType, RendererBackendFactoryFunction>;

FactoryMap* GetFactoryMap() {
  static FactoryMap factory_map;
  return &factory_map;
}

std::unique_ptr<RendererBackend>
CreateRendererBackend(RendererType type) {
  FactoryMap* factory_map = GetFactoryMap();
  auto it = factory_map->find(type);
  ASSERT_MSG(it != factory_map->end(),
             "Could not find renderer backend: %s",
             ToString(type));

  RendererBackendFactoryFunction factory = it->second;
  return factory();
}

}  // namespace

void SuscribeRendererBackendFactory(RendererType type,
                                    RendererBackendFactoryFunction factory) {
  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

// Init ------------------------------------------------------------------------

bool InitRenderer(Renderer* renderer, InitRendererConfig* config) {
  ASSERT(!Valid(renderer));
  ASSERT(config->type != RendererType::kLast);
  ASSERT(config->window);

  renderer->type = config->type;
  renderer->window = config->window;

  renderer->backend = CreateRendererBackend(renderer->type);
  if (!renderer->backend) {
    NOT_REACHED();
    LOG(ERROR, "Could not create backend for: %s", ToString(renderer->type));
    return false;
  }

  return renderer->backend->Init(renderer, config);
}

// Shutdown --------------------------------------------------------------------

Renderer::~Renderer() = default;

// Frame -----------------------------------------------------------------------

void StartFrame(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->StartFrame();
}

void EndFrame(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->EndFrame();
  WindowSwapBuffers(renderer->window);
}

// Shaders ---------------------------------------------------------------------

bool RendererParseShader(Renderer* renderer,
                         const std::string& vert_path,
                         const std::string& frag_path,
                         Shader* out) {
  ASSERT(Valid(renderer));
  return renderer->backend->ParseShader(vert_path, frag_path, out);
}

void RendererUnstageShader(Renderer* renderer, Shader* shader) {
  ASSERT(Valid(renderer));
  renderer->backend->UnstageShader(shader);
}

// Extras ----------------------------------------------------------------------

bool Valid(Renderer* renderer) {
  return renderer->type != RendererType::kLast && !!renderer->backend;
}

const char* ToString(RendererType type) {
  switch (type) {
    case RendererType::kOpenGL: return "OpenGL";
    case RendererType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

}  // namespace rothko
