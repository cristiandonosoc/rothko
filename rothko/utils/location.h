// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "rothko/utils/macros.h"

#define FROM_HERE ::rothko::Location{__FILE__, __LINE__, __FUNCTION__}

namespace rothko {

// Represents a single location within the code.
struct Location {
  const char* file = nullptr;
  int line = 0;
  const char* function = nullptr;
};

// Strips all the namespaces:
//
// std::__v2::(anon)::Foo::Bar -> Bar
std::string GetBaseFunction(const std::string&);

std::string ToString(Location);


}  // namespace rothko
