// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/graphics/common/renderer.h"

#include <map>

#include "rothko/utils/logging.h"

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


// Extras ----------------------------------------------------------------------

const char* ToString(RendererType type) {
  switch (type) {
    case RendererType::kOpenGL: return "OpenGL";
    case RendererType::kLast: return "Last";
  }

  NOT_REACHED();
  return "<unknown>";
}

}  // namespace rothko
