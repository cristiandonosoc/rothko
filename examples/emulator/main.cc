// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/platform/platform.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/file.h>
#include <rothko/math/math.h>

#include "display.h"
#include "memory.h"
#include "shader.h"
#include "quad.h"

using namespace rothko;
using namespace rothko::imgui;

namespace {

int kTileSize = 8;
Int2 kTileCount =  {16, 16 + 8};
Int2 kTextureDim = kTileCount * kTileSize;

Vec2 kUVOffset = {1.0f / 16.0f, 1.0f / (16.0f + 8.0f)};

uint32_t VecToColor(ImVec4 color) {
  // RGBA
  return ((uint8_t)(color.x * 255.0f) << 24) |
         ((uint8_t)(color.y * 255.0f) << 16) |
         ((uint8_t)(color.z * 255.0f) << 8) |
         ((uint8_t)(color.w * 255.0f));
}

Color TileColor(Int2 coord) {
  Color color{};
  if (IS_EVEN(coord.x + coord.y)) {
    return colors::kRed;
  } else {
    return colors::kGreen;
  }
}

Int2 IndexToCoord(int index) {
  return {index % kTileCount.x, (index / kTileCount.y)};
}

int CoordToIndex(Int2 coord) {
  return coord.y * kTileCount.x + coord.x;
}

void PaintTile(Color* data, Int2 coord, Color color) {
  int cx = coord.x * kTileSize;
  int cy = coord.y * kTileSize;

  for (int y = cy; y < cy + kTileSize; y++) {
    for (int x = cx; x < cx + kTileSize; x++) {
      data[y * kTextureDim.width + x] = color;
    }
  }
}

void PaintTile(Color* data, int index, Color color) {
  PaintTile(data, IndexToCoord(index), color);
}

void PaintTile(Color* data, Int2 coord, const Color* tile_data) {
  Color* tile_base = data + (coord.y * kTileSize * kTextureDim.x  + coord.x * kTileSize);
  for (int y = 0; y < 8; y++) {
    Color* row_base = tile_base + (y * kTextureDim.x);
    for (int x = 0; x < 8; x++) {
      row_base[x] = *tile_data++;
    }
  }
}

void PaintTile(Color* data, int index, const Color* tile_data) {
  PaintTile(data, IndexToCoord(index), tile_data);
}

}  // namespace

int main(int argc, char* argv[]) {
  bool log_to_stdout = false;
  if (argc > 1) {
    if (strcmp(argv[1], "--stdout") == 0)
      log_to_stdout = true;
  }

  printf("Log to stdout %d\n", log_to_stdout);

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

  // Create the background texture.
  Texture background_texture;
  background_texture.name = "background texture";
  background_texture.type = TextureType::kRGBA;
  background_texture.dims = kTextureDim;

  size_t size = sizeof(Color) * kTextureDim.width * kTextureDim.height;
  background_texture.data = (uint8_t*)malloc(size);
  background_texture.free_function = free;

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  if (!RendererStageTexture(game.renderer.get(), &background_texture, config))
    return 1;

  rothko::emulator::Memory memory = {};

  // Fill in the texture.
  Color tile_color[64];
  Color* base_color = (Color*)background_texture.data.value;

  /* int prev_index = -1; */
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

          memcpy(&memory, data.data(), KILOBYTES(64));

          LOG(App, "0x%x", *(uint32_t*)(memory.rom_bank0 + 0x104));


          for (int y = 0; y < 16 + 8; y++) {
            for (int x = 0; x < 16; x++) {
              rothko::emulator::Tile* tile = memory.vram.tiles + (y * 16) + x;
              rothko::emulator::TileToTexture(memory.mapped_io.bgp, tile, tile_color);
              PaintTile(base_color, {x, y}, tile_color);
            }
          }

          LOG(App, "Updating texture");
          RendererSubTexture(game.renderer.get(), &background_texture,
                             {0, 0}, kTextureDim,
                             background_texture.data.value);

          LOG(App, "Updating mesh");

          // Generate the background mesh.
          CreateBackgroundMesh(game.renderer.get(), &display, &memory, &background_texture,
                               normal_shader.get(), (uint8_t*)&normal_ubo);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    ImGui::ShowDemoWindow();

    CreateDisplayImgui(&memory, &background_texture);


    // window.
    {
      ImGui::Begin("Emulator");
      ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);

      ImGui::Separator();
      ImGui::Text("Tile texture");
      ImGui::Image(&background_texture, {200, 200 * 1.5f});


      ImGui::End();
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
