// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace rothko {

// Amount of nanoseconds since the program started.
// Implementation is in each platform implementation (<PLATFORM>_platform.cc).
// See rothko/platform/platform.h for more details.
uint64_t GetNanoseconds();

struct Time {
  // Represents the time since |Init was called.
  uint64_t total_time = 0;    // In nanoseconds.
  float seconds = 0;

  float frame_delta = 0;

  float frame_delta_average = 0;
  float frame_rate = 0;

  // Amount of frames to keep track of in order to get an average frame time.
  static constexpr int kFrameTimesCounts = 128;

  // TODO(Cristian): When we're interested, start tracking these times.
  float frame_deltas[kFrameTimesCounts] = {};
  int frame_deltas_index = 0;
};

Time InitTime();
void Update(Time*);

}  // namespace rothko
