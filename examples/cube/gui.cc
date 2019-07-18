// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "gui.h"

#include <rothko/logging/logging.h>
#include <rothko/ui/imgui.h>

using namespace rothko;

void CreateDebugGui() {

  /* auto& io = ImGui::GetIO(); */

  ImGui::SetNextWindowSize({1000, 400}, ImGuiCond_Once);

  ImGui::Begin("Logs", nullptr);
  /* auto window_size = ImGui::GetWindowSize(); */
  /* ImGui::SetWindowPos({0, io.DisplaySize.y - window_size.y}, ImGuiCond_Once); */

  auto* logs = LogContainer::Get();
  int write_index = logs->write_index;
  // No entry after this should be shown.
  /* uint64_t max_nanos = logs->entries[write_index % LogContainer::kMaxEntries].nanoseconds; */

  int read_index = write_index - LogContainer::kMaxEntries;
  if (read_index < 0)
    read_index = 0;

  for (int i = read_index; i < write_index; i++) {
    auto& entry = logs->entries[i % LogContainer::kMaxEntries];
    /* // If we reach an older entry, we're done. */
    /* if (entry.nanoseconds > max_nanos) */
    /*   break; */

    ImGui::Text("%s", LogCategoryToString(entry.log_category));
    ImGui::SameLine();
    ImGui::Text("%s", entry.location.c_str());
    ImGui::SameLine();
    ImGui::Text("%s", entry.msg.c_str());
  }


  /* auto* logs = LogContainer::Get(); */
  /* int max = logs->write_index % LogContainer::kMaxEntries; */

  /* LogEntry entry; */
  /* entry.location = ToString(FROM_HERE); */
  /* entry.nanoseconds = 123131; */
  /* entry.msg = "Some message"; */

  /* for (int i = 0; i < max; i++) { */
  /*   ImGui::Text("%s", LogCategoryToString(entry.log_category)); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%s", entry.location.c_str()); */
  /*   ImGui::SameLine(); */
  /*   ImGui::Text("%s", entry.msg.c_str()); */
  /* } */

  ImGui::End();
}

