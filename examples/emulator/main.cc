// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/platform/platform.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/file.h>

#include "display.h"
#include "memory.h"
#include "shader.h"

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

Color TileColor(Int2 coord) {
  Color color{};
  if (IS_EVEN(coord.x + coord.y)) {
    return colors::kRed;
  } else {
    return colors::kGreen;
  }
}

int kTileSize = 8;
Int2 kTileCount =  {16, 16 + 8};
Int2 kTextureDim = kTileCount * kTileSize;

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
  if (!InitGame(&game, &window_config, RendererType::kOpenGL, log_to_stdout))
    return 1;

  ImguiContext imgui;
  if (!InitImgui(&game.renderer, &imgui))
    return 1;

  /* std::string path = OpenFileDialog(); */
  /* LOG(App, "Got path: %s", path.c_str()); */

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Create the painting shader.
  auto normal_shader = rothko::emulator::CreateNormalShader(&game.renderer);
  if (!normal_shader) {
    ERROR(App, "Could not create shader.");
    return 1;
  }

  // Create the background texture.
  Texture texture;
  texture.name = "background texture";
  texture.type = TextureType::kRGBA;
  texture.dims = kTextureDim;

  size_t size = sizeof(Color) * kTextureDim.width * kTextureDim.height;
  texture.data = (uint8_t*)malloc(size);
  texture.free_function = free;

  // Fill in the texture.
  Color* base_color = (Color*)texture.data.value;
  Color* color = base_color;
  for (int y = 0; y < kTextureDim.height; y++) {
    /* int tile_y = (y / kTileSize); */

    for (int x = 0; x < kTextureDim.width; x++) {
      /* int tile_x = (x / kTileSize); */

      /* *color = TileColor({tile_x, tile_y}); */
      *color = 0xff000000;
      color++;
    }
  }

  constexpr uint8_t kTileData[] = {
    0b00'11'00'11,
    0b00'00'11'11,
    0b11'00'11'00,
    0b11'11'00'00,

    0b00'11'00'11,
    0b00'00'11'11,
    0b11'00'11'00,
    0b11'11'00'00,

    0b00'11'00'11,
    0b00'00'11'11,
    0b11'00'11'00,
    0b11'11'00'00,

    0b00'11'00'11,
    0b00'00'11'11,
    0b11'00'11'00,
    0b11'11'00'00,
  };
  static_assert(sizeof(kTileData) == 16);

  Color tile_color[64];
  /* rothko::emulator::TileToTexture(kTileData, tile_color); */

  /* PaintTile(base_color, {0, 0}, tile_color); */
  /* PaintTile(base_color, {1, 1}, tile_color); */
  /* PaintTile(base_color, {0, 2}, tile_color); */
  /* PaintTile(base_color, {2, 3}, tile_color); */
  /* PaintTile(base_color, {3, 3}, tile_color); */




  /* PaintTile(base_color, {0, 0}, colors::kBlue); */
  /* PaintTile(base_color, {1, 1}, colors::kBlue); */
  /* PaintTile(base_color, {1, 4}, colors::kBlue); */
  /* PaintTile(base_color, {6, 6}, colors::kBlue); */
  /* PaintTile(base_color, 0, colors::kBlue); */
  /* PaintTile(base_color, 11, colors::kBlue); */
  /* PaintTile(base_color, 41, colors::kBlue); */
  /* PaintTile(base_color, 66, colors::kBlue); */

  int prev_index = -1;
  uint8_t* color_end = (uint8_t*)color;
  ASSERT(color_end == texture.data.value + size);

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  if (!RendererStageTexture(config, &game.renderer, &texture))
    return 1;

  uint64_t step = kSecond;
  uint64_t next_time = GetNanoseconds() + step;


  rothko::emulator::Memory memory;


  bool running = true;
  while (running) {
    auto events = Update(&game);
    for (auto event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    auto frame_time = GetNanoseconds();
    if (frame_time >= next_time) {
      next_time = next_time + step;
      int new_index = Random(0, 100);
      LOG(App, "Recover index %d (%s), change index %d (%s)",
               prev_index, ToString(IndexToCoord(prev_index)).c_str(),
               new_index, ToString(IndexToCoord(new_index)).c_str());

      if (prev_index > 0) {
        /* PaintTile((Color*)texture.data.value, prev_index, TileColor(IndexToCoord(prev_index))); */
      }
      prev_index = new_index;

      /* PaintTile((Color*)texture.data.value, new_index, colors::kBlue); */

      /* RendererSubTexture(&game.renderer, &texture, {0, 0}, kTextureDim, texture.data.value); */
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

          for (int y = 0; y < 16 + 8; y++) {
            for (int x = 0; x < 16; x++) {
              rothko::emulator::Tile* tile = memory.vram.tiles + (y * 16) + x;
              rothko::emulator::TileToTexture(memory.mapped_io.bgp, tile, tile_color);
              PaintTile(base_color, {x, y}, tile_color);
            }
          }

          RendererSubTexture(&game.renderer, &texture, {0, 0}, kTextureDim, texture.data.value);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }



    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
    // window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.

      ImGui::Text(
          "This is some useful text.");  // Display some text (you can use a format strings too)

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

      if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true
                                    // when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);

      ImGui::Image(&texture, {800, 800 + 400});
      ImGui::End();
    }

    PerFrameVector<RenderCommand> commands;

    // Clear command.
    ClearFrame clear_frame;
    clear_frame = {};
    clear_frame.color = VecToColor(clear_color);
    commands.push_back(std::move(clear_frame));

    auto imgui_commands = EndFrame(&imgui);
    commands.insert(commands.end(), imgui_commands.begin(), imgui_commands.end());

    RendererExecuteCommands(commands, &game.renderer);

    EndFrame(&game.renderer);
  }
}
