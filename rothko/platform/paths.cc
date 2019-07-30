// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/platform/paths.h"

#include "rothko/utils/macros.h"
#include "rothko/utils/strings.h"

namespace rothko {

// TODO(Cristian): Use std::filesystem (C++17) for this eventually.
std::string JoinPaths(const std::vector<std::string>& paths) {
  return Join(paths, "/");
}

std::string GetBasename(const std::string& path) {
  size_t separator = path.rfind(FILEPATH_SEPARATOR);
  if (separator == std::string::npos)
    return path;
  return path.substr(separator + 1);
}

}  // namespace rothko

