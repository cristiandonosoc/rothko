// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "rothko/graphics/graphics.h"

namespace rothko {

struct LightWidgetManager {
  struct PointLight {
    Vec3 position;
    Vec3 color;
  };

  struct DirectionalLight {
    Vec3 position;
    Vec3 rotation;
    Vec3 color;
  };

  std::string name;

  Mesh* point_light_mesh = nullptr;
  Shader* point_light_shader = nullptr;

  Mesh* directional_light_mesh = nullptr;

  std::vector<PointLight> point_lights;
  std::vector<DirectionalLight> directional_lights;
};

Shader CreatePointLightShader(Renderer*);

Mesh CreatePointLightMesh(Renderer*);
Mesh CreateDirectionalLightMesh(Renderer*);

void Init(LightWidgetManager*, const std::string& name,
                               Shader* point_light_shader,
                               Mesh* point_light_mesh);
void Reset(LightWidgetManager*);

void PushPointLight(LightWidgetManager*, LightWidgetManager::PointLight);
void PushDirectionalLight(LightWidgetManager*, LightWidgetManager::DirectionalLight);

std::vector<RenderMesh> GetRenderCommands(const LightWidgetManager&);

}  // namespace

