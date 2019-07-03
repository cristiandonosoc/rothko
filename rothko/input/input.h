// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "rothko/math/math.h"
#include "rothko/utils/logging.h"

namespace rothko {

#define GET_KEY(key) ((uint8_t)::rothko::Key::k##key)

enum class Key : uint32_t {
  kUp, kDown, kLeft, kRight,
  kA, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM, kN, kEnhe /* Ñ */, kO, kP,
  kQ, kR, kS, kT, kU, kV, kW, kX, kY, kZ,
  k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,
  kBackquote,
  kPageUp, kPageDown, kHome, kEnd, kInsert, kDelete,
  kBackspace, kSpace, kEnter, kEscape,
  kTab, kCtrl, kAlt, kShift, kSuper /* windows key */,
  kLast,  // Not a key, used to verify the input buffer size.
};
const char* ToString(Key);

constexpr uint32_t kMaxKeys = 128;
static_assert((uint32_t)Key::kLast < kMaxKeys);

// Represents the input state for a frame.
// Currently there is only support for one set of input.
struct Input {
  bool down_last_frame[kMaxKeys] = {};
  bool down_this_frame[kMaxKeys] = {};
  static_assert(sizeof(down_last_frame) == sizeof(down_this_frame));

  // These are equal to down_this_frame[<ARROW KEY>] == true.
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;

  struct MouseState {
    Int2 pos;
    Int2 wheel;   // x: Horizontal. y: Vertical.

    bool left = false;
    bool middle = false;
    bool right = false;
  };

  MouseState prev_mouse;
  MouseState mouse;
  Int2 mouse_offset;
};

void NewFrame(Input*);

bool KeyDown(Input* input, Key key);
bool KeyDownThisFrame(Input* input, Key key);
bool KeyUpThisFrame(Input* input, Key key);

}  // rothko
