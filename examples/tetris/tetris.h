// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <rothko/math/math.h>

#include <memory>

namespace rothko {

struct Input;
struct RenderCommand;
struct Renderer;

}  // namespace rothko

namespace tetris {

constexpr uint32_t kTetrisSizeX = 6;
constexpr uint32_t kTetrisSizeY = 10;
constexpr uint32_t kTetrisTotalY = 14;  // Surplus to put new shapes in.

struct TetrisImpl;

// Boards.
BIT_MASK(CurrentShape, 0, 0b111);

// Blocks.
BIT_FLAG(BlockPresence, 0);
BIT_MASK(BlockType, 1, 0b111); // Presence without BlockType means a already set block.

static_assert(kCurrentShapeMask == kBlockTypeMask);

struct Tetris {
  ~Tetris();

  uint32_t flags = 0;
  ::rothko::Int2 shape_pos;

  // Board entry.
  // Bit 0: Whether there is a block.
  uint32_t board[kTetrisSizeX * kTetrisTotalY] = {};

  TetrisImpl* impl = nullptr;
};

inline uint32_t GetBlock(const Tetris& tetris, uint32_t x, uint32_t y) {
  return tetris.board[y * kTetrisSizeX + x];
}
inline void SetBlock(Tetris* tetris, uint32_t x, uint32_t y, uint32_t block) {
  tetris->board[y * kTetrisSizeX + x] = block;
}

std::unique_ptr<Tetris> InitTetris(rothko::Renderer*);

rothko::RenderCommand Update(Tetris*, rothko::Renderer*, const rothko::Input&);

}  // namespace tetris

