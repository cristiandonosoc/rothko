// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "rothko/ui/imgui/imgui_windows.h"

#include "rothko/ui/imgui.h"

namespace rothko {
namespace imgui {

void CreateLogWindow() {
  ImGui::SetNextWindowSize({1500, 400});

  ImGui::Begin("Logs", nullptr);

  ImGui::BeginChild("Logchild");

  auto& logs = GetLogs();

  // HEADER
  ImGui::Text(
      "%15s | %10s | %25s | %4s | %25s | MESSAGE", "TIME", "CATEGORY", "FILE", "LINE", "FUNCTION");
  ImGui::Separator();

  int write_index = logs.write_index;
  if (write_index == 0)
    return;
  int read_index = write_index - LogContainer::kMaxEntries;
  if (read_index < 0)
    read_index = 0;

  // Write the last one first.
  for (int i = write_index - 1; i >= read_index; i--) {
    auto& entry = logs.entries[i % LogContainer::kMaxEntries];
    /* if (entry.log_time.nanos > last_nano) */
    /*   continue; */

    ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    if (entry.severity == LogSeverity::kWarning) {
      color = {1.0f, 1.0f, 0.0f, 1.0f};
    } else if (entry.severity == LogSeverity::kError) {
      color = {1.0f, 0.0f, 0.0f, 1.0f};
    }

    auto& time = entry.log_time;
    ImGui::TextColored(color,
                       "%02d:%02d:%02d:%06d | %10s | %25s | %4d | %25s | %s",
                       time.hours,
                       time.minutes,
                       time.seconds,
                       time.micros,
                       ToString(entry.category),
                       entry.location.file,
                       entry.location.line,
                       entry.location.function,
                       entry.msg.c_str());
  }

  ImGui::EndChild();
  ImGui::End();
}

}  // namespace imgui
}  // namespace rothko
