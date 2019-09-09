// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/math/math.h>
#include <rothko/platform/platform.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/file.h>
#include <rothko/utils/strings.h>
#include <third_party/imgui_extras/imgui_memory_editor.h>

#include "disassembler.h"
#include "gameboy.h"
#include "quad.h"
#include "shader.h"
#include "textures.h"

using namespace rothko;
using namespace rothko::imgui;
using namespace rothko::emulator;

namespace {

uint32_t VecToColor(ImVec4 color) {
  // RGBA
  return ((uint8_t)(color.x * 255.0f) << 24) |
         ((uint8_t)(color.y * 255.0f) << 16) |
         ((uint8_t)(color.z * 255.0f) << 8) |
         ((uint8_t)(color.w * 255.0f));
}

void LoadROM(Gameboy* gameboy) {
  std::string path = OpenFileDialog();
  Catridge catridge;
  if (!Load(&catridge, path)) {
    ERROR(App, "Could not read ROM %s", path.c_str());
    return;
  }

  gameboy->catridge = std::move(catridge);
}

void LoadDump(Game* game, Gameboy* gameboy) {
  std::string path = OpenFileDialog();
  std::vector<uint8_t> data;
  if (!ReadWholeFile(path, &data)) {
    ERROR(App, "Could not read file %s", path.c_str());
    return;
  }

  ASSERT_MSG(data.size() >= KILOBYTES(64), "Got size %zu", data.size());

  memcpy(&gameboy->memory, data.data(), KILOBYTES(64));

  UpdateTextures(game->renderer.get(), &gameboy->memory, &gameboy->textures);
  Disassemble(gameboy->memory, &gameboy->disassembler);
}

}  // namespace

int main(int argc, char* argv[]) {
  bool log_to_stdout = false;
  if (argc > 1) {
    if (strcmp(argv[1], "--stdout") == 0)
      log_to_stdout = true;
  }

  printf("Log to stdout %d\n", log_to_stdout);

  bool memory_loaded = false;

  Game game;
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, log_to_stdout))
    return 1;

  ImguiContext imgui;
  if (!InitImgui(game.renderer.get(), &imgui))
    return 1;

  rothko::emulator::Display display;
  if (!InitDisplay(&game, &display))
    return 1;

  LOG(App, "Window: %s", ToString(game.window.screen_size).c_str());

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Create the painting shader.
  rothko::emulator::NormalUBO normal_ubo;
  normal_ubo.proj = Ortho(0.0f, (float)game.window.screen_size.width,
                          (float)game.window.screen_size.height, 0.0f);
  /* float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height; */
  /* normal_ubo.proj = Perspective(ToRadians(60.0f), aspect_ratio, 0.1f, 100.0f); */
  /* normal_ubo.view = LookAt({5, 5, 5}, {}, {0, 1, 0}); */
  normal_ubo.view = Mat4::Identity();

  auto normal_shader = rothko::emulator::CreateNormalShader(game.renderer.get());
  if (!normal_shader) {
    ERROR(App, "Could not create shader.");
    return 1;
  }

  Gameboy gameboy = {};
  if (!Init(&game, &gameboy)) {
    ERROR(App, "Coult not initialize gameboy.");
    return 1;
  }

  bool running = true;
  while (running) {
    auto events = Update(&game);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (KeyUpThisFrame(&game.input, Key::kEscape)) {
      running = false;
      break;
    }

    StartFrame(&imgui, &game.window, &game.time, &game.input);

    CreateLogWindow();

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Load ROM")) {
          LoadROM(&gameboy);
        }

        if (ImGui::MenuItem("Load Dump")) {
          LoadDump(&game, &gameboy);
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Options")) {
        ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color
        ImGui::EndMenu();
      }

      auto framerate_str = StringPrintf("Application average %.3f ms/frame (%.1f FPS)",
                                        1000.0f / ImGui::GetIO().Framerate,
                                        ImGui::GetIO().Framerate);
      float framerate_str_width = framerate_str.length() * imgui.font_size.x;

      ImGui::GetWindowWidth();

      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - framerate_str_width);
      ImGui::Text("%s", framerate_str.c_str());

      ImGui::EndMainMenuBar();
    }

    if (memory_loaded) {
      static MemoryEditor memory_editor;
      memory_editor.DrawWindow("Memory Editor", &gameboy.memory, sizeof(gameboy.memory));
    }

    ImGui::ShowDemoWindow();

    CreateDisplayImgui(&gameboy.memory, &gameboy.textures);

    if (Valid(gameboy.catridge)) {
      ImGui::Begin("Catridge");
      ImGui::LabelText("Title", "%s", gameboy.catridge.title.c_str());
      ImGui::LabelText("Gameboy Type", "%s", ToString(gameboy.catridge.gameboy_type));
      ImGui::LabelText("Catridge Type", "%s", ToString(gameboy.catridge.catridge_type));
      ImGui::LabelText("ROM Size (bytes)", "%u", gameboy.catridge.rom_size);
      ImGui::LabelText("RAM Size (bytes)", "%u", gameboy.catridge.ram_size);

      ImGui::End();
    }

    // Disassembler window.
    {
      /* if (Valid(disassembler)) { */
      /*   ImGui::Begin("Disassemble"); */

      /*   for (auto& [opcode, dis_inst] : disassembler.instructions) { */
      /*     auto& inst = dis_inst.instruction; */
      /*     if (!IsCBInstruction(inst)) { */
      /*       ImGui::Text("0x%x: %s (0x%x), LENGTH: %u, TICKS: %u", */
      /*                   dis_inst.address, */
      /*                   GetName(dis_inst.instruction), */
      /*                   inst.opcode.low, */
      /*                   inst.length, */
      /*                   inst.ticks); */
      /*     } else { */
      /*       ImGui::Text("0x%x: %s (0x%x), LENGTH: %u, TICKS: %u", */
      /*                   dis_inst.address, */
      /*                   GetName(dis_inst.instruction), */
      /*                   inst.opcode.opcode, */
      /*                   inst.length, */
      /*                   inst.ticks); */
      /*     } */
      /*   } */

      /*   ImGui::End(); */
      /* } */
    }

    PerFrameVector<RenderCommand> commands;

    // Clear command.
    ClearFrame clear_frame;
    clear_frame = {};
    clear_frame.color = VecToColor(clear_color);
    commands.push_back(std::move(clear_frame));

    // Config Renderer.
    ConfigRenderer config_renderer;
    // TODO(Cristian): Actually find the height of the bar.
    config_renderer.viewport_base = {0, 0};
    config_renderer.viewport_size = game.window.screen_size - Int2{0, 20};
    commands.push_back(std::move(config_renderer));

    if (!display.quads.render_commands.empty())
      commands.insert(commands.end(), display.quads.render_commands.begin(),
                                      display.quads.render_commands.end());

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));
    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
