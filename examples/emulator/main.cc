// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/ui/imgui.h>

#include <rothko/platform/dialogs.h>

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

int main() {
  Game game;
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, RendererType::kOpenGL))
    return 1;

  ImguiContext imgui;
  if (!InitImgui(&game.renderer, &imgui))
    return 1;

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Create the background texture.
  Texture texture;
  texture.name = "background texture";
  texture.type = TextureType::kRGBA;
  texture.dims = {200, 200};

  size_t size = sizeof(Color) * 200 * 200;
  texture.data = (uint8_t*)malloc(size);
  texture.free_function = free;

  // Fill in the texture.
  Color* color = (Color*)texture.data.value;
  for (int y = 0; y < 200; y++) {
    int tile_y = (y / 20) % 2;

    for (int x = 0; x < 200; x++) {
      int tile_x = (x / 20) % 2;
      * color = {};
      color->a = 0xff;
      if ((tile_y + tile_x) % 2 == 0) {
        color->r = 0xff;
      } else {
        color->g = 0xff;
      }

      color++;
    }
  }

  uint8_t* color_end = (uint8_t*)color;
  ASSERT(color_end == texture.data.value + size);

  StageTextureConfig config = {};
  config.generate_mipmaps = false;
  config.min_filter = StageTextureConfig::Filter::kNearest;
  config.max_filter = StageTextureConfig::Filter::kNearest;
  if (!RendererStageTexture(config, &game.renderer, &texture))
    return 1;


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

      ImGui::Image(&texture, {(float)texture.dims.width, (float)texture.dims.height});
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
