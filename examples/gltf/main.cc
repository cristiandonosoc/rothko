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

#include "scene.h"
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

struct ModelContext {
  std::vector<std::unique_ptr<Model>> models;
  int selected_model = -1;

  std::vector<ModelInstance> instances;
  int selected_instance = -1;
};

void InstanceModificationWindow(ModelContext* model_context) {
  ImGui::Begin("Selected Model");

  ImGui::Text("Instance Count: %zu", model_context->instances.size());
  ImGui::Text("Selected instance: %d", model_context->selected_instance);

  if (model_context->selected_instance != -1) {
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

    TransformImguiWidget(model_context->instances[model_context->selected_instance].transform);
  }

  if (ImGui::CollapsingHeader("Instances")) {
    for (int i = 0; i < (int)model_context->instances.size(); i++) {
      ImGui::PushID(i);

      auto& instance = model_context->instances[i];

      bool selected = model_context->selected_instance == i;
      if (ImGui::Selectable(instance.model->path.c_str(), selected))
        model_context->selected_instance = i;

      ImGui::PopID();
    }
  }

  ImGui::End();
}

bool StageModel(Renderer* renderer, Model* model) {
  for (auto& mesh : model->meshes) {
    if (!RendererStageMesh(renderer, mesh.get()))
      return false;
  }

  for (auto& texture : model->textures) {
    if (!RendererStageTexture(renderer, texture.get()))
      return false;
  }

  return true;
}

void ModelSelectionWindow(ModelContext* model_context) {
  ImGui::Begin("Models");

  ImGui::Text("Selected model: %d", model_context->selected_model);
  if (ImGui::Button("Instantiate")) {
    ModelInstance instance = {};
    Update(&instance.transform);

    instance.model = model_context->models[model_context->selected_model].get();
    model_context->instances.push_back(std::move(instance));
    model_context->selected_instance = (int)model_context->instances.size() - 1;

    model_context->selected_model = -1;
  }

  // Selected model section.
  if (ImGui::CollapsingHeader("Selected Model")) {
    if (model_context->selected_model != -1) {
      auto& model = model_context->models[model_context->selected_model];
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
  }

  // Model selection.

  if (ImGui::CollapsingHeader("Models")) {
    for (int model_index = 0; model_index < (int)model_context->models.size(); model_index++) {
      ImGui::PushID(model_index);
      auto& model = model_context->models[model_index];
      /* if (ImGui::Selectable(model->path.c_str()), model_context->selected_model == model_index) { */
      bool selected = model_context->selected_model == model_index;
      if (ImGui::Selectable(model->path.c_str(), selected))
        model_context->selected_model = model_index;
      ImGui::PopID();
    }
  }

  ImGui::End();
}

std::vector<RenderCommand> CreateSelectedModelCommands(const ModelContext& model_context,
                                                       const Shader* model_shader) {
  if (model_context.selected_model == -1)
    return {};

  static auto stack_allocator = CreateStackAllocatorFor<ModelTransform>(1024);
  Reset(&stack_allocator);

  static ModelInstance instance = {};
  instance.model = model_context.models[model_context.selected_model].get();

  std::vector<RenderCommand> commands;
  for (auto& node : instance.model->nodes) {
    // Calculate the transform.
    auto* model_transform = Allocate<ModelTransform>(&stack_allocator);
    model_transform->transform = instance.transform.world_matrix * node.transform.world_matrix;
    model_transform->inverse_transform = Transpose(Inverse(model_transform->transform));

    for (const ModelPrimitive& primitive : node.primitives) {
      if (!Valid(primitive))
        continue;

      // Render the mesh!
      RenderMesh render_mesh = {};
      render_mesh.mesh = primitive.mesh;
      render_mesh.shader = model_shader;
      render_mesh.primitive_type = PrimitiveType::kTriangles;
      render_mesh.indices_count = primitive.mesh->indices.size();
      SetWireframeMode(&render_mesh.flags);

      render_mesh.ubo_data[0] = (uint8_t*)model_transform;
      render_mesh.ubo_data[1] = (uint8_t*)&primitive.material->base_color;

      commands.push_back(std::move(render_mesh));

      /* Vec3 m1 = ToVec3(transform_data->transform * primitive.bounds.min); */
      /* Vec3 m2 = ToVec3(transform_data->transform * primitive.bounds.max); */

      /* auto [min, max] = GetBounds(m1, m2); */
      /* PushCube(lines, min, max, Color::Black()); */
    }
  }

  return commands;
}

std::vector<RenderCommand> CreateInstancesCommands(const PushCamera& camera,
                                                   ModelContext* model_context,
                                                   const Shader* model_shader) {
  static auto stack_allocator = CreateStackAllocatorFor<ModelTransform>(1024);
  Reset(&stack_allocator);

  std::vector<RenderCommand> commands;
  for (int instance_index = 0; instance_index < (int)model_context->instances.size();
       instance_index++) {
    auto& instance = model_context->instances[instance_index];

    if (model_context->selected_instance == instance_index) {
      TransformWidget(gWidgetOperation, gTransformKind, camera, &instance.transform);
      Update(&instance.transform);
    }

    for (auto& node : instance.model->nodes) {
      auto* model_transform = Allocate<ModelTransform>(&stack_allocator);
      model_transform->transform = instance.transform.world_matrix * node.transform.world_matrix;
      model_transform->inverse_transform = Transpose(Inverse(model_transform->transform));

      for (const ModelPrimitive& primitive : node.primitives) {
        if (!Valid(primitive))
          continue;

        // Render the mesh!
        RenderMesh render_mesh = {};
        render_mesh.mesh = primitive.mesh;
        render_mesh.shader = model_shader;
        render_mesh.primitive_type = PrimitiveType::kTriangles;
        render_mesh.indices_count = primitive.mesh->indices.size();

        render_mesh.ubo_data[0] = (uint8_t*)model_transform;
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

  ModelContext model_context = {};
  for (auto& dir_entry : dir_entries) {
    if (dir_entry.is_dir)
      continue;

    auto model = std::make_unique<Model>();
    if (!gltf::LoadModel(dir_entry.path, model.get()))
      return 1;

    /* if (!StageModel(game.renderer.get(), model.get())) */
    /*   return 1; */

    model_context.models.push_back(std::move(model));
    break;
  }

  printf("Loaded %u models.\n", (uint32_t)model_context.models.size());
  printf("Loaded %u meshes.\n", (uint32_t)model_context.models[0]->meshes.size());

  gltf::Scene scene = {};
  scene.models = std::move(model_context.models);
  if (!gltf::SerializeScene(scene, "scene.rtk"))
    return 1024;

  gltf::Scene read_scene = gltf::ReadScene("scene.rtk");


  return 0;

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

  model_context.selected_model = 2;

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
    ModelSelectionWindow(&model_context);
    InstanceModificationWindow(&model_context);

    // Create Commands.

    PerFrameVector<RenderCommand> commands;
    commands.push_back(ClearFrame::FromColor(Color::Graycc()));
    auto push_camera = GetPushCamera(camera);
    commands.push_back(push_camera);

    PushCommands(&commands, CreateSelectedModelCommands(model_context, model_shader.get()));
    PushCommands(&commands, CreateInstancesCommands(push_camera, &model_context,
                                                    model_shader.get()));

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
