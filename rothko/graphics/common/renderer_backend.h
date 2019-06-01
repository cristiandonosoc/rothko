// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

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

struct RendererBackend {
  virtual ~RendererBackend() = default;

  virtual bool Init(InitRendererConfig*);
};

}  // namespace rothko
