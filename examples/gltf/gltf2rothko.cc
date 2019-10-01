// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>

using namespace rothko;

int main() {
  auto log_handle = InitLoggingSystem(true);

  LOG(App, "Hello");
}
