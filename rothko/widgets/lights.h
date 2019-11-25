// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "rothko/graphics/graphics.h"

#include "rothko/widgets/lines.h"

namespace rothko {

struct Transform;

struct PointLight {
  Transform* transform = nullptr;
  Vec3 color = {};
};

struct DirectionalLight {
  Transform* transform = nullptr;
  Vec3 color = {};
};

struct SpotLight {
  Transform* transform = nullptr;
  float angle;
  Color color;
};

struct LightWidgetManager {
  std::string name;

  const Shader* point_light_shader = nullptr;
  const Shader* directional_light_shader = nullptr;

  Mesh point_light_mesh;
  Mesh directional_light_mesh;

  std::vector<PointLight> point_lights;
  std::vector<DirectionalLight> directional_lights;

  LineManager lines;
};

// Uses default shaders and meshes.
bool Init(LightWidgetManager*, Renderer*, const std::string& name);

bool Init(LightWidgetManager*, Renderer*, const std::string& name,
          const Shader* point_light_shader,
          const Shader* directional_light_shader,
          const Shader* lines_shader);

void Reset(LightWidgetManager*);
void Stage(LightWidgetManager*, Renderer*);

// Transforms must be stable in memory throughout the frame.
void PushPointLight(LightWidgetManager*, Transform*, Vec3 color);
void PushDirectionalLight(LightWidgetManager*, Transform*, Vec3 color);
void PushSpotLight(LightWidgetManager*, const SpotLight&);

std::vector<RenderCommand> GetRenderCommands(const LightWidgetManager&);

}  // namespace

