// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <rothko/game.h>
#include <rothko/graphics/default_shaders/default_shaders.h>
#include <rothko/math/math.h>
#include <rothko/memory/stack_allocator.h>
#include <rothko/models/gltf/loader.h>
#include <rothko/models/model.h>
#include <rothko/scene/camera.h>
#include <rothko/ui/imgui.h>
#include <rothko/utils/strings.h>
#include <rothko/widgets/grid.h>
#include <rothko/widgets/widgets.h>
#include <stdio.h>
#include <third_party/tiny_gltf/tiny_gltf.h>

#include "shaders.h"

using namespace rothko;

namespace {

static WidgetOperation gWidgetOperation = WidgetOperation::kTranslate;
static TransformKind gTransformKind = TransformKind::kGlobal;

std::pair<Vec3, Vec3> GetBounds(const Vec3& m1, const Vec3& m2) {
  Vec3 min = {};
  Vec3 max = {};

  min.x = Min(m1.x, m2.x);
  min.y = Min(m1.y, m2.y);
  min.z = Min(m1.z, m2.z);

  max.x = Max(m1.x, m2.x);
  max.y = Max(m1.y, m2.y);
  max.z = Max(m1.z, m2.z);

  return {min, max};
}

std::vector<RenderCommand> CreateRenderCommands(const PushCamera& camera,
                                                std::vector<ModelInstance>* instances,
                                                LineManager* lines,
                                                const Shader* model_shader,
                                                int index) {
  (void)lines;
  static auto stack_allocator = CreateStackAllocatorFor<ModelTransform>(1024);
  Reset(&stack_allocator);
  std::vector<RenderCommand> commands;

  // Put in a model instance.
  for (int i = 0; i < (int)instances->size(); i++) {
    if (i != index)
      continue;

    auto& instance = (*instances)[i];

    // Add a transformation widget.
    TransformWidget(gWidgetOperation, gTransformKind, camera, &instance.transform);
    Update(&instance.transform);

    for (auto& node : instance.model->nodes) {
      // Calculate the transform.
      auto* transform_data = Allocate<ModelTransform>(&stack_allocator);
      transform_data->transform = instance.transform.world_matrix * node.transform.world_matrix;
      transform_data->inverse_transform = Transpose(Inverse(transform_data->transform));

      for (const ModelPrimitive& primitive : node.primitives) {
        if (!Valid(primitive))
          continue;

        // Render the mesh!
        RenderMesh render_mesh = {};
        render_mesh.mesh = primitive.mesh;
        render_mesh.shader = model_shader;
        render_mesh.primitive_type = PrimitiveType::kTriangles;
        render_mesh.indices_count = primitive.mesh->indices.size();

        render_mesh.ubo_data[0] = (uint8_t*)transform_data;
        render_mesh.ubo_data[1] = (uint8_t*)&primitive.material->base_color;

        commands.push_back(std::move(render_mesh));

        /* Vec3 m1 = ToVec3(transform_data->transform * primitive.bounds.min); */
        /* Vec3 m2 = ToVec3(transform_data->transform * primitive.bounds.max); */

        /* auto [min, max] = GetBounds(m1, m2); */
        /* PushCube(lines, min, max, Color::Black()); */
      }
    }
  }

  return commands;
}

}  // namespace

int main(int argc, const char* argv[]) {
  Game game = {};
  InitWindowConfig window_config = {};
  window_config.type = WindowType::kSDLOpenGL;
  window_config.resizable = true;
  /* window_config.fullscreen = true; */
  window_config.screen_size = {1920, 1440};
  if (!InitGame(&game, &window_config, true))
    return 1;

  std::string path;
  if (argc == 2)
    path = argv[1];


  if (path.empty()) {
    path = OpenFileDialog();
    if (path.empty()) {
      ERROR(App, "Could not get model path.");
      return 1;
    }
  }

  LOG(App, "Path: %s", path.c_str());
  std::vector<DirectoryEntry> dir_entries;
  if (!ListDirectory(GetBasePath(path), &dir_entries, "gltf")) {
    dir_entries.push_back({false, path});
  }


  std::vector<std::unique_ptr<Model>> models;
  for (auto& dir_entry : dir_entries) {
    if (dir_entry.is_dir)
      continue;

    auto model = std::make_unique<Model>();
    if (!gltf::LoadModel(dir_entry.path, model.get()))
      return 1;
    models.push_back(std::move(model));
  }

  // -----------------------------------------------------------------------------------------------

  std::vector<ModelInstance> instances;
  instances.reserve(models.size());
  for (uint32_t i = 0; i < models.size(); i++) {
    auto& model = models[i];
    for (auto& mesh : model->meshes) {
      if (!RendererStageMesh(game.renderer.get(), mesh.get()))
        return 1;
    }

    for (auto& texture : model->textures) {
      if (!RendererStageTexture(game.renderer.get(), texture.get()))
        return 1;
    }

    auto& instance = instances.emplace_back();
    instance.model = model.get();
    /* uint32_t x = (i % 5) * 10; */
    /* uint32_t z = (i / 5) * 10; */

    /* instance.transform.position = {(float)x, 0, (float)z}; */
    Update(&instance.transform);
  }

  // Model instances -------------------------------------------------------------------------------


  Grid grid;
  if (!Init(&grid, game.renderer.get()))
    return 1;

  auto model_shader = gltf::CreateModelShader(game.renderer.get());
  if (!model_shader)
    return 1;

  float aspect_ratio = (float)game.window.screen_size.width / (float)game.window.screen_size.height;
  OrbitCamera camera = OrbitCamera::FromLookAt({5, 5, 5}, {}, ToRadians(60.0f), aspect_ratio);

  LineManager lines = {};
  if (!Init(&lines, game.renderer.get(), "lines"))
    return 1;

  /* Mat4 model_transform = Mat4::Identity(); */

  imgui::ImguiContext imgui;
  if (!Init(game.renderer.get(), &imgui))
    return 1;

  bool running = true;
  bool rotate = false;
  float time = 0;
  float angle = 0;
  while (running) {
    WindowEvent event = StartFrame(&game);
    if (event == WindowEvent::kQuit) {
      running = false;
      break;
    }

    if (KeyUpThisFrame(game.input, Key::kEscape)) {
      running = false;
      break;
    }

    Reset(&lines);
    DefaultUpdateOrbitCamera(game.input, &camera);



    if (KeyUpThisFrame(game.input, Key::kSpace))
      rotate = !rotate;

    if (rotate) {
      time += game.time.frame_delta;
      angle = ToRadians(20.0f) * time;
    }
    /* model_transform = Rotate({0, 1, 0}, angle); */

    // Imgui.

    Update(&imgui, &game);

    ImGui::Begin("Models");

    ImGui::Text("Manipulation");

    ImGui::RadioButton("Translate", (int*)&gWidgetOperation, (int)WidgetOperation::kTranslate);
    ImGui::SameLine();
    ImGui::RadioButton("Rotate", (int*)&gWidgetOperation, (int)WidgetOperation::kRotate);
    ImGui::SameLine();
    ImGui::RadioButton("Scale", (int*)&gWidgetOperation, (int)WidgetOperation::kScale);

    ImGui::RadioButton("World", (int*)&gTransformKind, (int)TransformKind::kGlobal);
    ImGui::SameLine();
    ImGui::RadioButton("Local", (int*)&gTransformKind, (int)TransformKind::kLocal);

    // Scale only works in local mode. Otherwise it resets the rotation.
    if (gWidgetOperation == WidgetOperation::kScale)
      gTransformKind = TransformKind::kLocal;

    ImGui::Separator();

    static int selected = -1;
    for (int model_index = 0; model_index < (int)models.size(); model_index++) {
      auto& model = models[model_index];
      int mesh_count = 0;
      mesh_count++;
      ImGui::PushID(mesh_count);

      ImGui::SetNextTreeNodeOpen(selected == model_index);
      if (ImGui::CollapsingHeader(model->name.c_str())) {
        selected = model_index;

        if (ImGui::CollapsingHeader("Nodes")) {
          for (uint32_t i = 0; i < model->nodes.size(); i++) {
            ImGui::PushID(i);
            if (i > 0)
              ImGui::Separator();

            TransformImguiWidget(model->nodes[i].transform);
            ImGui::PopID();
          }
        }

        if (ImGui::CollapsingHeader("Meshes")) {
          for (uint32_t i = 0; i < model->meshes.size(); i++) {
            ImGui::PushID(i);
            if (i > 0)
              ImGui::Separator();

            auto& mesh = model->meshes[i];
            ImGui::Text("%s", mesh->name.c_str());
            ImGui::Text("Vertices: %u (%zu bytes)", mesh->vertex_count, mesh->vertices.size());
            ImGui::Text("Indices: %zu (%zu bytes)",
                        mesh->indices.size(),
                        mesh->indices.size() / sizeof(Mesh::IndexType));

            ImGui::PopID();
          }
        }

        if (ImGui::CollapsingHeader("Materials")) {
          int count = 0;
          for (auto& material : model->materials) {
            ImGui::PushID(count);
            if (count++ > 0)
              ImGui::Separator();
            if (material->base_texture)
              ImGui::Text("Base Texture: %s", material->base_texture->name.c_str());
            ImGui::ColorEdit4("Base Color", (float*)&material->base_color);
            ImGui::PopID();
          }
        }
      }
      ImGui::PopID();
    }

    ImGui::End();

    // Create Commands.

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    auto push_camera = GetPushCamera(camera);
    commands.push_back(push_camera);

    PushCommands(
        &commands,
        CreateRenderCommands(push_camera, &instances, &lines, model_shader.get(), selected));

    commands.push_back(grid.render_command);
    if (!Stage(&lines, game.renderer.get()))
        return 2;

    auto cmd = GetRenderCommand(lines);
    commands.push_back(GetRenderCommand(lines));

    PushCommands(&commands, EndFrame(&imgui));

    commands.push_back(PopCamera());

    RendererExecuteCommands(game.renderer.get(), std::move(commands));
    RendererEndFrame(game.renderer.get(), &game.window);
  }
}
