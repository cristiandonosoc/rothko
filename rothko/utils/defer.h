// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <functional>

#include "rothko/utils/macros.h"

namespace rothko {

// Run a function on the end of the scope.
// Usage:
//
// auto defer = Defer([]() { <SOME FUNCTION> });

struct DeferInternal {
  DeferInternal(std::function<void()> f) : func(std::move(f)) {}
  ~DeferInternal() {
    if (func)
      func();
  }

  DELETE_COPY_AND_ASSIGN(DeferInternal);

  DeferInternal(DeferInternal&& rhs) : func(std::move(rhs.func)) {
    rhs.func = {};
  }

  DeferInternal& operator=(DeferInternal&& rhs) {
    if (this == &rhs)
      return *this;

    this->func = std::move(rhs.func);
    rhs.func = {};
    return *this;
  }

 private:
  std::function<void()> func;

  friend DeferInternal Defer(std::function<void()>);
};

inline DeferInternal Defer(std::function<void()> f) {
  return DeferInternal(std::move(f));
}

}  // namespace rothko
