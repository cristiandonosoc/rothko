// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

namespace rothko {

// Returns a concatenate of path parts joined together (normally by '/').
std::string JoinPaths(const std::vector<std::string>& paths);

// Strip everything up to and including the last slash.
std::string GetBasename(const std::string& path);

}  // namespace rothko
