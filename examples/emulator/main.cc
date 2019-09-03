// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/math/math.h>
#include <rothko/platform/platform.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/file.h>
#include <third_party/imgui_extras/imgui_memory_editor.h>

#include "disassembler.h"
#include "gameboy.h"
#include "quad.h"
#include "shader.h"
#include "textures.h"

using namespace rothko;
using namespace rothko::imgui;

namespace {

uint32_t VecToColor(ImVec4 color) {
  // RGBA
  return ((uint8_t)(color.x * 255.0f) << 24) |
         ((uint8_t)(color.y * 255.0f) << 16) |
         ((uint8_t)(color.z * 255.0f) << 8) |
         ((uint8_t)(color.w * 255.0f));
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

  auto textures = rothko::emulator::CreateTextures(&game);
  if (!textures) {
    ERROR(App, "Could not create textures.");
    return 1;
  }

  rothko::emulator::Gameboy gameboy = {};
  rothko::emulator::Disassembler disassembler = {};

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
        if (ImGui::MenuItem("Open")) {
          std::string path = OpenFileDialog();
          std::vector<uint8_t> data;
          if (!ReadWholeFile(path, &data)) {
            ERROR(App, "Could not read %s", path.c_str());
          }

          ASSERT_MSG(data.size() >= KILOBYTES(64), "Got size %zu", data.size());

          memory_loaded = true;

          memcpy(&gameboy.memory, data.data(), KILOBYTES(64));

          LOG(App, "0x%x", *(uint32_t*)(gameboy.memory.rom_bank0 + 0x104));

          UpdateTileTexture(&gameboy.memory, &textures->tiles);
          UpdateBackgroundTexture(&gameboy.memory, &textures->background);
          UpdateWindowTexture(&gameboy.memory, &textures->window);
          UpdateSpritesDebugTexture(&gameboy.memory, &textures->sprites_debug);

          RendererSubTexture(game.renderer.get(), &textures->tiles);
          RendererSubTexture(game.renderer.get(), &textures->background);
          RendererSubTexture(game.renderer.get(), &textures->window);
          RendererSubTexture(game.renderer.get(), &textures->sprites_debug);

          // Generate the background mesh.
          CreateBackgroundMesh(game.renderer.get(), &display, &gameboy.memory, &textures->tiles,
                               normal_shader.get(), (uint8_t*)&normal_ubo);

          Disassemble(gameboy, &disassembler);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    if (memory_loaded) {
      static MemoryEditor memory_editor;
      memory_editor.DrawWindow("Memory Editor", &gameboy.memory, sizeof(gameboy.memory));
    }

    ImGui::ShowDemoWindow();

    CreateDisplayImgui(&gameboy.memory, textures.get());

    // window.
    {
      ImGui::Begin("Emulator");
      ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);

      ImGui::Separator();
      ImGui::Text("Tile texture");
      ImGui::Image(&textures->tiles, {200, 200 * 1.5f});

      ImGui::End();
    }

    // Disassembler window.
    {
      if (Valid(disassembler)) {
        ImGui::Begin("Disassemble");

        for (auto& [opcode, dis_inst] : disassembler.instructions) {
          auto& inst = dis_inst.instruction;
          if (!IsCBInstruction(inst)) {
            ImGui::Text("0x%x: OPCODE 0x%x, LENGTH: %u, TICKS: %u",
                        dis_inst.address,
                        inst.opcode.low,
                        inst.length,
                        inst.ticks);
          } else {
            ImGui::Text("0x%x: OPCODE 0x%x, LENGTH: %u, TICKS: %u",
                        dis_inst.address,
                        inst.opcode.opcode,
                        inst.length,
                        inst.ticks);
          }
        }

        ImGui::End();
      }
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
