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
struct Mesh;
struct Renderer;
struct Shader;
struct StageTextureConfig;
struct Texture;

struct RendererBackend {
  RendererBackend() = default;
  virtual ~RendererBackend() = default;
  // Renderer backends don't move!
  DELETE_COPY_AND_ASSIGN(RendererBackend);
  DELETE_MOVE_AND_ASSIGN(RendererBackend);

  virtual bool Init(Renderer*, InitRendererConfig*) = 0;

  virtual void StartFrame() = 0;
  virtual void EndFrame() = 0;

  // Meshes.
  virtual bool StageMesh(Mesh*) = 0;
  virtual void UnstageMesh(Mesh*) = 0;

  // Shaders.
  virtual bool StageShader(Shader*) = 0;
  virtual void UnstageShader(Shader*) = 0;

  // Textures.
  virtual bool StageTexture(const StageTextureConfig&, Texture*) = 0;
  virtual void UnstageTexture(Texture*) = 0;
};

}  // namespace rothko
