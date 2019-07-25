// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

struct Timings {
  uint64_t start_frame = 0;
  uint64_t create_my_commands = 0;
  uint64_t create_imgui_commands = 0;
  uint64_t execute_commands = 0;
  uint64_t end_frame = 0;

  float frame_delta = 0.0f;
  int event_count = 0;
};

// Assumes Imgui is correctly set already.
void CreateDebugGui(const Timings&);


