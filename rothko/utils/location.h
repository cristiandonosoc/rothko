// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "rothko/utils/macros.h"



namespace rothko {

// Represents a single location within the code.
struct Location {
  const char* file = nullptr;
  int line = 0;
  const char* function = nullptr;
};

constexpr const char* StrAfterToken(const char* const str,
                                    const char* const last_token,
                                    const char token) {
  if (*str == 0)
    return last_token;

  if (*str == token) {
    return StrAfterToken(str + 1, str + 1, token);
  } else {
    return StrAfterToken(str + 1, last_token, token);
  }
}

constexpr const char* StrAfterToken(const char* const str, const char token) {
  return StrAfterToken(str, str, token);
}

#define FROM_HERE                                                                         \
  ::rothko::Location {                                                                    \
    StrAfterToken(__FILE__, FILEPATH_SEPARATOR), __LINE__, StrAfterToken(__FUNCTION__, ':') \
  }

// Strips all the namespaces:
//
// std::__v2::(anon)::Foo::Bar -> Bar
std::string GetBaseFunction(const std::string&);

std::string ToString(Location);


}  // namespace rothko
