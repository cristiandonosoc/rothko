// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/utils/macros.h"

#define FROM_HERE ::warhol::Location{__FILE__, __LINE__, __FUNCTION__}

namespace rothko {

// Represents a single location within the code.
struct Location {
  const char* file = nullptr;
  int line = 0;
  const char* function = nullptr;
};

}  // namespace rothko
