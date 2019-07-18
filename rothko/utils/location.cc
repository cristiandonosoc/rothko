// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/location.h"

#include "rothko/utils/strings.h"

namespace rothko {

std::string ToString(Location location) {
  return StringPrintf("%s:%d (%s)", location.file, location.line, location.function);
}

}  // namespace rothko
