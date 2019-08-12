// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <rothko/platform/platform.h>

namespace rothko {

struct Timer {
  static Timer CreateAndStart() {
    Timer timer;
    timer.Start();
    return timer;
  }

  void Start() {
    nanos = GetNanoseconds();
  }

  uint64_t End() {
    return GetNanoseconds() - nanos;
  }

  uint64_t nanos = 0;
};

}  // namespace rothko
