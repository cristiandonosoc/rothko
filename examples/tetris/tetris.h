// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/math/math.h>

#include <memory>

namespace rothko {

struct Input;
struct Game;
struct Renderer;
struct RenderCommand;

}  // namespace rothko

namespace tetris {

constexpr uint32_t kTetrisSizeX = 10;
constexpr uint32_t kTetrisSizeY = 20;
constexpr uint32_t kTetrisTotalY = kTetrisSizeY + 4;  // Surplus to put new shapes in.

struct TetrisImpl;

// Boards.
BIT_MASK(CurrentShape, 0, 4);
BIT_MASK(NextShape, 4, 3);
BIT_MASK(CurrentRotation, 7, 2);

// Blocks.
BIT_FLAG(BlockPresence, 0);
BIT_MASK(BlockType, 1, 4);  // Presence without BlockType means a already set block.
BIT_FLAG(ShapeOrigin, 5);

static_assert(kCurrentShapeMask == kBlockTypeMask);

struct Tetris {
  ~Tetris();

  uint32_t flags = 0;
  ::rothko::Int2 shape_pos;


  // Board entry.
  // Bit 0: Whether there is a block.
  uint32_t board[kTetrisSizeX * kTetrisTotalY] = {};

  // Timing in seconds.
  float time_move_down = 0.8f;
  float last_move_down = 0;

  float time_press_down = 0.06f;
  float last_press_down = 0;

  float time_move_side = 0.5f;
  float last_move_side = 0;

  TetrisImpl* impl = nullptr;
};

inline uint32_t GetBlock(const Tetris& tetris, uint32_t x, uint32_t y) {
  return tetris.board[y * kTetrisSizeX + x];
}
inline void SetBlock(Tetris* tetris, uint32_t x, uint32_t y, uint32_t block) {
  tetris->board[y * kTetrisSizeX + x] = block;
}

std::unique_ptr<Tetris> InitTetris(rothko::Renderer*);

::rothko::RenderCommand Update(Tetris*, ::rothko::Game* game);

}  // namespace tetris

