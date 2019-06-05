// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/utils/macros.h"

#include <string>

namespace rothko {

// RendererBackend
// =============================================================================
//
// Abstract interface each specific graphics API integration must provide for it
// to work in Rothko. Each integration has to subclass |RendererBackend| and
// suscribe a factory function, keyed by its RendererType.
//
// At the moment of needing a particular backend, the code will call that
// factory function to obtain an instance of that particular WindowBackend.
//
// It is recommended that the suscription is done at initialization time, so
// that the backend is assured to be there without any further work from part of
// the called.

struct InitRendererConfig;
struct Renderer;
struct Shader;

struct RendererBackend {
  RendererBackend() = default;
  virtual ~RendererBackend() = default;
  // Renderer backends don't move!
  DELETE_COPY_AND_ASSIGN(RendererBackend);
  DELETE_MOVE_AND_ASSIGN(RendererBackend);

  virtual bool Init(Renderer*, InitRendererConfig*) = 0;

  virtual void StartFrame() = 0;
  virtual void EndFrame() = 0;

  // Shader.
  virtual bool ParseShader(const std::string& vert_path,
                           const std::string& frag_path,
                           Shader* out) = 0;
  virtual void UnstageShader(Shader*) = 0;
};

}  // namespace rothko
