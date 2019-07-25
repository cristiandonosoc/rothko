// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "gui.h"

#include <rothko/logging/logging.h>
#include <rothko/platform/paths.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/macros.h>

using namespace rothko;

namespace {

struct LogTime {
  int hours;
  int minutes;
  int seconds;
  /* int milliseconds; */
  int micro;
};

LogTime NanoToTime(uint64_t nanos) {
  LogTime time = {};


  /* uint64_t micro = nanos / 1000; */
  /* time.microseconds = nanos - micro; */
  /* uint64_t milli = micro / 1000; */
  /* time.milliseconds = micro - milli; */
  /* uint64_t secs = milli / 1000; */

  uint64_t micros = nanos / 1000;
  time.micro = micros % 1000000;
  uint64_t secs = micros / 1000000;
  time.seconds = secs % 60;
  uint64_t min = secs / 60;
  time.minutes = min % 60;
  time.hours = min / 60;

  return time;
}

float ToSeconds(uint64_t nanos) {
  uint64_t micros = nanos / 1000;
  return (float)micros / 1000000.0f;
}

}  // namespace

void CreateDebugGui(const Timings& timings) {
  ImGui::SetNextWindowSize({1000, 400}, ImGuiCond_Once);

  ImGui::Begin("Logs", nullptr);

  static float scroll_y = 0.0f;
  /* float scroll_y_prev = scroll_y; */
  static bool lock_scrolling = true;

  /* bool pressed = ImGui::Checkbox("Scroll to bottom", &lock_scrolling); */
  ImGui::SameLine();
  ImGui::Text("SCROLL: %f", scroll_y);
  ImGui::SameLine();
  ImGui::Text("LOCK: %d", lock_scrolling);
  ImGui::Separator();

  ImGui::LabelText("Start frame", "%f", ToSeconds(timings.start_frame));
  ImGui::LabelText("Create commands", "%f", ToSeconds(timings.create_my_commands));
  ImGui::LabelText("Create IMGUI commands", "%f", ToSeconds(timings.create_imgui_commands));
  ImGui::LabelText("Execute commands", "%f", ToSeconds(timings.execute_commands));
  ImGui::LabelText("End Frame", "%f", ToSeconds(timings.end_frame));

  ImGui::Separator();

  /* ImGui::BeginChild("Logchild"); */

  /* auto* logs = LogContainer::Get(); */
  /* int write_index = logs->write_index; */

  /* int read_index = write_index - LogContainer::kMaxEntries; */
  /* if (read_index < 0) */
  /*   read_index = 0; */

  /* for (int i = read_index; i < write_index; i++) { */
  /*   auto& entry = logs->entries[i % LogContainer::kMaxEntries]; */
  /*   LogTime time = NanoToTime(entry.nanoseconds); */

  /*   ImGui::Text("%02d:%02d:%02d.%06d", */
  /*               time.hours, */
  /*               time.minutes, */
  /*               time.seconds, */
  /*               time.micro); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%8s", LogCategoryToString(entry.log_category)); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%25s", GetBasename(entry.location.file).c_str()); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%4d", entry.location.line); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%25s", GetBaseFunction(entry.location.function).c_str()); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%s", entry.msg.c_str()); */
  /* } */

  /* scroll_y = ImGui::GetScrollY(); */

  /* if (pressed) { */
  /*   ImGui::SetScrollHereY(1.0f); */
  /* } else { */
  /*   if (scroll_y < scroll_y_prev) { */
  /*     lock_scrolling = false; */
  /*   } else if (lock_scrolling) { */
  /*     ImGui::SetScrollHereY(1.0f); */
  /*   } */
  /* } */

  /* ImGui::EndChild(); */
  ImGui::End();
}

