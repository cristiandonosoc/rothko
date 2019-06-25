// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

namespace rothko {

// Vector that uses per-frame storage.
// It is a stack allocator.
template <typename T>
using PerFrameVector = std::vector<T>;  // TODO(Cristian): Use allocator.

}  // namespace rothko
