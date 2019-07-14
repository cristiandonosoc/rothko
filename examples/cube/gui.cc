// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "gui.h"

#include <rothko/ui/imgui.h>

void CreateGui() {

  auto& io = ImGui::GetIO();

  ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y * 0.4f});

  ImGui::Begin("Logs", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
  auto window_size = ImGui::GetWindowSize();
  ImGui::SetWindowPos({0, io.DisplaySize.y - window_size.y});

  ImGui::Columns(2, nullptr, true);

  for (int i = 0; i < 100; i++) {
    ImGui::Text("INFO");
  }

  ImGui::NextColumn();
  for (int i = 0; i < 100; i++) {
    ImGui::Text("Some debug entry.");
  }





  ImGui::End();
}

