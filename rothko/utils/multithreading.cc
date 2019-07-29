// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/utils/multithreading.h"

namespace rothko {

LockGuard::LockGuard(std::mutex* mutex) : mutex_(mutex) {
  if (mutex_)
    mutex_->lock();
}

LockGuard::~LockGuard() {
  if (mutex_)
    mutex_->unlock();
}

UnlockGuard::UnlockGuard(std::mutex* mutex) : mutex_(mutex) {
  if (mutex_)
    mutex_->unlock();
}

UnlockGuard::~UnlockGuard() {
  if (mutex_)
    mutex_->lock();
}

}  // namespace rothko
