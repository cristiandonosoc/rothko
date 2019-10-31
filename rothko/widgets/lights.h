// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "rothko/graphics/graphics.h"

namespace rothko {

struct Transform;

struct LightWidgetManager {
  struct Light {
    Transform* transform = {};
    Vec3 color = {};
  };

  std::string name;

  Mesh* point_light_mesh = nullptr;
  Shader* point_light_shader = nullptr;

  Mesh* directional_light_mesh = nullptr;
  Shader* directional_light_shader = nullptr;

  std::vector<Light> point_lights;
  std::vector<Light> directional_lights;
};

Shader CreatePointLightShader(Renderer*);
Mesh CreatePointLightMesh(Renderer*);

Shader CreateDirectionalLightShader(Renderer*);
Mesh CreateDirectionalLightMesh(Renderer*);

void Init(LightWidgetManager*, const std::string& name,
                               Shader* point_light_shader,
                               Mesh* point_light_mesh,
                               Shader* directional_light_shader,
                               Mesh* directional_light_mesh);
void Reset(LightWidgetManager*);

// Transforms must be stable in memory throughout the frame.
void PushPointLight(LightWidgetManager*, Transform*, Vec3 color);
void PushDirectionalLight(LightWidgetManager*, Transform*, Vec3 color);

/* void PushPointLight(LightWidgetManager*, LightWidgetManager::PointLight); */
/* void PushDirectionalLight(LightWidgetManager*, LightWidgetManager::DirectionalLight); */

std::vector<RenderMesh> GetRenderCommands(const LightWidgetManager&);

}  // namespace

