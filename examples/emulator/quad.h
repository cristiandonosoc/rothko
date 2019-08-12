// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "rothko/graphics/graphics.h"

namespace rothko {

struct QuadManager {
  const char* name;
  Mesh mesh;

  // Every push will either expand the previous render command or create a new one.
  // This list won't be cleared until |Reset| has been called explicitly on a QuadManager.
  std::vector<RenderCommand> render_commands;
  int index_offset = 0;     // Index of the next index to be inserted to a render command.

  // Whether the current state of quads is staged.
  // When there are no quads (|render_commands| is empty), |staged| is true.
  bool staged = false;

  // How many quads this manager can hold.
  int capacity = 0;
  // How many quads are loaded.
  int size = 0;
};

// |capacity| indicates how many |QuadEntry| this quad manager can support at the same time.
struct QuadManagerConfig {
  const char* name = "quad-manager";
  int capacity = 1000;
};
bool Init(Renderer*, QuadManager* out, const QuadManagerConfig&);

// Some other value could be used, but |capacity| should only be set when |Init| has been called.
inline bool Valid(const QuadManager& q) { return q.capacity != 0; }



void Reset(QuadManager*);

// Push will immediatelly append a render command into |render_commands|. It will try to batch quads
// together if they share exactly the same render command data. This means that if you push the same
// command data into a QuadManager, it will result into only one big render command.
//
// The quad inserted will be a a convention from a AABB:
//
//  from --> 1 + + + + + + + + + + + 2
//           +\                      +\
//           + \                     + \
//           +  \                    +  \
//           +   \                   +   \
//           +    3 + + + + + + + + +++ + 4
//           +    +                  +    +
//           +    +                  +    +
//           +    +                  +    +
//           +    +                  +    +
//           +    +                  +    +
//           +    +                  +    +
//           5 + +++ + + + + + + + + 6    +
//            \   +                   \   +
//             \  +                    \  +
//              \ +                     \ +
//               \+                      \+
//                7 + + + + + + + + + + + 8 <-- to
//
// Given the |from| and |to| vertices, the quad generated will be the CW (1, 2, 7, 8).
// UV coordinates will be interpolated as such.
struct QuadEntry {
  // Vertex data.
  Vec3 from_pos;
  Vec3 to_pos;
  Vec2 from_uv;
  Vec2 to_uv;
  Color color;

  // Render command data.
  // TODO(Cristian): Support more than one texture here.
  Shader* shader = nullptr;     // Required.
  Texture* texture = nullptr;   // Required.
  uint8_t* vert_ubo = nullptr;
  uint8_t* frag_ubo = nullptr;
};
void Push(QuadManager*, const QuadEntry&);

void Stage(Renderer*, QuadManager*);

}  // namespace

