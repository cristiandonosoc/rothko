// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/input/input.h"

#include <cstring>

#include "rothko/logging/logging.h"

namespace rothko {

void NewFrame(Input* input) {
  input->event_count = 0;

  // Move the down buttons over.
  std::memcpy(input->down_last_frame, input->down_this_frame,
              sizeof(input->down_this_frame));

  // Actual control state.
  input->up = false;
  input->down = false;
  input->left = false;
  input->right = false;

  input->prev_mouse = input->mouse;
  input->mouse = {};
  input->mouse_offset = {};
}
bool KeyDown(const Input& input, Key key) {
  int val = (int)key;
  ASSERT(val < (int)Key::kLast);
  return input.down_this_frame[val];
}

bool KeyDownThisFrame(const Input& input, Key key) {
  int val = (int)key;
  ASSERT(val < (int)Key::kLast);
  return input.down_this_frame[val] && !input.down_last_frame[val];
}

bool KeyUpThisFrame(const Input& input, Key key) {
  int val = (int)key;
  ASSERT(val < (int)Key::kLast);
  return !input.down_this_frame[val] && input.down_last_frame[val];
}

const char* ToString(Key key) {
  switch (key) {
    case Key::kUp: return "Up";
    case Key::kDown: return "Down";
    case Key::kLeft: return "Left";
    case Key::kRight: return "Right";
    case Key::kA: return "A";
    case Key::kB: return "B";
    case Key::kC: return "C";
    case Key::kD: return "D";
    case Key::kE: return "E";
    case Key::kF: return "F";
    case Key::kG: return "G";
    case Key::kH: return "H";
    case Key::kI: return "I";
    case Key::kJ: return "J";
    case Key::kK: return "K";
    case Key::kL: return "L";
    case Key::kM: return "M";
    case Key::kN: return "N";
    case Key::kEnhe: return "Enhe";
    case Key::kO: return "O";
    case Key::kP: return "P";
    case Key::kQ: return "Q";
    case Key::kR: return "R";
    case Key::kS: return "S";
    case Key::kT: return "T";
    case Key::kU: return "U";
    case Key::kV: return "V";
    case Key::kW: return "W";
    case Key::kX: return "X";
    case Key::kY: return "Y";
    case Key::kZ: return "Z";
    case Key::k0: return "0";
    case Key::k1: return "1";
    case Key::k2: return "2";
    case Key::k3: return "3";
    case Key::k4: return "4";
    case Key::k5: return "5";
    case Key::k6: return "6";
    case Key::k7: return "7";
    case Key::k8: return "8";
    case Key::k9: return "9";
    case Key::kBackquote: return "`";
    case Key::kPageUp: return "PageUp";
    case Key::kPageDown: return "PageDown";
    case Key::kHome: return "Home";
    case Key::kEnd: return "End";
    case Key::kInsert: return "Insert";
    case Key::kDelete: return "Delete";
    case Key::kBackspace: return "Backspace";
    case Key::kSpace: return "Space";
    case Key::kEnter: return "Enter";
    case Key::kEscape: return "Escape";
    case Key::kTab: return "Tab";
    case Key::kCtrl: return "Ctrl";
    case Key::kAlt: return "Alt";
    case Key::kShift: return "Shift";
    case Key::kSuper: return "Super";
    case Key::kLast: return "Last";
  }

  NOT_REACHED();
  return nullptr;
}

}  // namespace rothko
