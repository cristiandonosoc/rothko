// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <mutex>

#include "rothko/utils/macros.h"

namespace rothko {

// If |mutex| is null, these locks will no-op.

struct LockGuard {
  LockGuard(std::mutex* mutex);
  ~LockGuard();
  DELETE_COPY_AND_ASSIGN(LockGuard);
  DELETE_MOVE_AND_ASSIGN(LockGuard);

 private:
  std::mutex* mutex_ = nullptr;
};

// |mutex| must be locked.
struct UnlockGuard {
  UnlockGuard(std::mutex* mutex);
  ~UnlockGuard();
  DELETE_COPY_AND_ASSIGN(UnlockGuard);
  DELETE_MOVE_AND_ASSIGN(UnlockGuard);

 private:
  std::mutex* mutex_ = nullptr;
};

}  // namespace


