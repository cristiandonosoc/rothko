// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <rothko/graphics/commands.h>
#include <rothko/input/input.h>
#include <rothko/utils/defer.h>
#include <rothko/widgets/lines.h>

using namespace rothko;

namespace tetris {

struct TetrisImpl {
  LineManager lines;
};

struct Shape {
  Int2 offsets[4];
};

const Shape kShapes[] = {
    {},                             // Empty.
    {Int2{0, 0}, Int2{0, 1}, Int2{1, 0}, Int2{1, 1}},
};
static_assert(std::size(kShapes) < kCurrentShapeMask);
static_assert(std::size(kShapes) < kBlockTypeMask);

// Init --------------------------------------------------------------------------------------------

namespace {

// Upside down!
// clang-format off
constexpr uint32_t kTestBoard[kTetrisSizeX * kTetrisSizeY] = {
  /*      0  1  2  3  4  5 */
  /* 0 */ 1, 0, 0, 0, 0, 0,
  /* 1 */ 0, 1, 0, 0, 0, 0,
  /* 2 */ 0, 0, 1, 0, 0, 0,
  /* 3 */ 0, 0, 0, 1, 0, 0,
  /* 4 */ 0, 0, 0, 1, 1, 0,
  /* 5 */ 0, 0, 0, 0, 0, 1,
  /* 6 */ 0, 0, 0, 0, 0, 0,
  /* 7 */ 0, 0, 0, 0, 0, 0,
  /* 8 */ 0, 0, 0, 0, 0, 0,
  /* 9 */ 0, 0, 0, 0, 0, 0,
};
// clang-format on

} // namespace

std::unique_ptr<Tetris> InitTetris(Renderer* renderer) {
  auto tetris = std::make_unique<Tetris>();
  for (uint32_t i = 0; i < std::size(kTestBoard); i++) {
    tetris->board[i] = kTestBoard[i];
  }

  tetris->impl = new TetrisImpl();
  if (!Init(&tetris->impl->lines, renderer, "tetris-lines"))
    return nullptr;

  return tetris;
}

Tetris::~Tetris() {
  if (impl) {
    impl->~TetrisImpl();
    free(impl);
  }
}

// Update ------------------------------------------------------------------------------------------

namespace {

std::pair<const Shape&, Int2> GetShape(const Tetris& tetris) {
  uint32_t shape_index = GetCurrentShape(tetris.flags);
  ASSERT(shape_index);
  const Shape& shape = kShapes[shape_index];

  return {shape, tetris.shape_pos};
}

// Returns true if a new shape was created.
bool CheckForNewShape(Tetris* tetris) {
  if (GetCurrentShape(tetris->flags))
    return false;

  // Choose a new shape randomly.
  int index = Random(1, std::size(kShapes) - 1);
  const Shape& shape = kShapes[index];
  SetCurrentShape(&tetris->flags, index);

  // Choose random place to place it.
  int place_index = Random(0, kTetrisSizeX - 2);
  tetris->shape_pos.x = place_index;
  tetris->shape_pos.y = kTetrisSizeY;

  // Mark the new places with this shape.
  for (auto& offset : shape.offsets) {
    uint32_t block = 0;
    SetBlockPresence(&block);
    SetBlockType(&block, index);
    SetBlock(tetris, place_index + offset.x, kTetrisSizeY + offset.y, block);
  }

  return true;
}

// Check that all the shapes don't clash with an already existing, non-shape block.
bool VerifyShapeCollision(Tetris* tetris, const Shape& shape, Int2 offset) {
  for (Int2 shape_offset : shape.offsets) {
    Int2 pos = tetris->shape_pos + shape_offset + offset;

    uint32_t block = GetBlock(*tetris, pos.x, pos.y);
    // If there is no block there, it's a valid move.
    if (!GetBlockPresence(block))
      continue;

    // We need to check that it is effectively a non-shape block. That is BlockType == 0.
    if (!GetBlockType(block))
      return false;
  }

  return true;
}

void MoveShapeBlocks(Tetris* tetris, const Shape& shape, Int2 offset) {
  // Perform the move. First save the values with a clear, then perform the move.
  uint32_t blocks[ARRAY_SIZE(Shape::offsets)];
  for (uint32_t i = 0; i < std::size(shape.offsets); i++) {
    Int2 pos = tetris->shape_pos + shape.offsets[i];
    blocks[i] = GetBlock(*tetris, pos.x, pos.y);
    SetBlock(tetris, pos.x, pos.y, 0);
  }

  // Now move the saved values.
  for (uint32_t i = 0; i < std::size(shape.offsets); i++) {
    Int2 pos = tetris->shape_pos + shape.offsets[i] + offset;
    SetBlock(tetris, pos.x, pos.y, blocks[i]);
  }

  tetris->shape_pos += offset;
}

enum class MoveResult {
  kNone,
  kMove,
  kCollision,
};

MoveResult MoveShapeDown(Tetris* tetris) {
  uint32_t shape_index = GetCurrentShape(tetris->flags);
  ASSERT(shape_index);
  const Shape& shape = kShapes[shape_index];

  if (!VerifyShapeCollision(tetris, shape, {0, -1}))
    return MoveResult::kCollision;

  MoveShapeBlocks(tetris, shape, {0, -1});
  return MoveResult::kMove;
}

void DoShapeCollision(Tetris* tetris) {
  const auto& [shape, shape_pos] = GetShape(*tetris);

  // We mark each current shape as a dead block.
  for (uint32_t i = 0; i < std::size(shape.offsets); i++) {
    Int2 pos = tetris->shape_pos + shape.offsets[i];

    uint32_t block = GetBlock(*tetris, pos.x, pos.y);
    SetBlockType(&block, 0);
    SetBlock(tetris, pos.x, pos.y, block);
  }

  // There is no more shape.
  SetCurrentShape(&tetris->flags, 0);
  tetris->shape_pos = {};
}


MoveResult MoveShape(Tetris* tetris, const Input& input) {
  uint32_t shape_index = GetCurrentShape(tetris->flags);
  ASSERT(shape_index);
  const Shape& shape = kShapes[shape_index];

  int offsetX = 0;
  if (KeyUpThisFrame(input, Key::kLeft)) {
    offsetX = -1;
    for (uint32_t i = 0; i < std::size(shape.offsets); i++) {
      int posX = tetris->shape_pos.x + shape.offsets[i].x + offsetX;
      if (posX < 0)
        return MoveResult::kNone;
    }
  } else if (KeyUpThisFrame(input, Key::kRight)) {
    offsetX = 1;
    for (uint32_t i = 0; i < std::size(shape.offsets); i++) {
      int posX = tetris->shape_pos.x + shape.offsets[i].x + offsetX;
      if (posX >= (int)kTetrisSizeX)
        return MoveResult::kNone;
    }
  } else if (KeyUpThisFrame(input, Key::kDown)) {
    MoveResult result = MoveShapeDown(tetris);
    if (result == MoveResult::kCollision)
      DoShapeCollision(tetris);
    return result;
  }

  // No key pressed.
  if (offsetX == 0)
    return MoveResult::kNone;

  if (!VerifyShapeCollision(tetris, shape, {offsetX, 0}))
    return MoveResult::kNone;

  MoveShapeBlocks(tetris, shape, {offsetX, 0});
  return MoveResult::kMove;
}

void MoveBlocksDown(Tetris* tetris, uint32_t row) {
  /* if (GetCurrentShape(tetris->flags)) */
  /*   tetris->shape_pos.y--; */
  for (uint32_t y = Max(row, 1u); y < kTetrisTotalY; y++) {
    for (uint32_t x = 0; x < kTetrisSizeX; x++) {
      uint32_t block = GetBlock(*tetris, x, y);
      if (!block)
        continue;

      // We only bring down "dead" blocks.
      if (GetBlockType(block))
        continue;


      // Get the block underneath. If there is one already, we cannot move down.
      uint32_t lower_block = GetBlock(*tetris, x, y - 1);
      if (lower_block)
        continue;

      // Move the block over.
      SetBlock(tetris, x, y - 1, block);
      SetBlock(tetris, x, y, 0);
    }
  }
}

void CheckForCompleteRows(Tetris* tetris) {
  uint32_t y = 0;
  while (y < kTetrisSizeY) {
    // Check for complete row.
    bool complete = true;
    for (uint32_t x = 0; x < kTetrisSizeX; x++) {
      if (!GetBlock(*tetris, x, y)) {
        complete = false;
        break;
      }
    }

    // If this row is not complete, we look for the next one.
    if (!complete) {
      y++;
      continue;
    }

    // The row is complete. We remove it and move the blocks down.
    for (uint32_t x = 0; x < kTetrisSizeX; x++) {
      SetBlock(tetris, x, y, 0);
    }
    MoveBlocksDown(tetris, y);

    // We recheck the row (not update |y|).
  }
}

RenderCommand RenderTetris(Tetris* tetris, Renderer* renderer) {
  TetrisImpl* impl = tetris->impl;

  // Bounds.
  Reset(&impl->lines);
  PushCube(&impl->lines, Vec3::Zero(), {kTetrisSizeX, kTetrisSizeY, 1}, Color::Black());
  PushCube(&impl->lines, {0, kTetrisSizeY, 0}, {kTetrisSizeX, kTetrisTotalY, 1},
           Color::LightGray());

  // Grid
  for (uint32_t x = 1; x < kTetrisSizeX; x++) {
    float xf = (float)x;
    PushCube(&impl->lines, {xf, 0, 0}, {xf, kTetrisSizeY, 1}, Color::Blue());
    PushCube(&impl->lines, {xf, kTetrisSizeY, 0}, {xf, kTetrisTotalY, 1}, Color::LightGray());
  }

  for (uint32_t y = 1; y < kTetrisSizeY; y++) {
    float yf = (float)y;
    PushCube(&impl->lines, {0, yf, 0}, {kTetrisSizeX, yf, 1}, Color::Blue());
  }

  for (uint32_t y = kTetrisSizeY; y < kTetrisTotalY; y++) {
    float yf = (float)y;
    PushCube(&impl->lines, {0, yf, 0}, {kTetrisSizeX, yf, 1}, Color::LightGray());
  }

  // Squares.
  constexpr float kBorder = 0.1f;
  for (uint32_t y = 0; y < kTetrisTotalY; y++) {
    for (uint32_t x = 0; x < kTetrisSizeX; x++) {
      uint32_t block = GetBlock(*tetris, x, y);
      if (!block)
        continue;

      uint32_t type = GetBlockType(block);

      PushCube(&impl->lines,
               Vec3(x + kBorder, y + kBorder, 0),
               Vec3(x + 1 - kBorder, y + 1 - kBorder, 1),
               type ? Color::Green() : Color::Red());
    }
  }

  if (!Stage(&impl->lines, renderer))
    return {};
  return GetRenderCommand(impl->lines);
}

}  // namespace

RenderCommand Update(Tetris* tetris, Renderer* renderer, const Input& input) {
  ASSERT(tetris->impl);

  // We see if we need to create a new shape.
  if (CheckForNewShape(tetris))
    return RenderTetris(tetris, renderer);

  if (MoveResult result = MoveShape(tetris, input); result != MoveResult::kNone) {
    if (result == MoveResult::kCollision)
      CheckForCompleteRows(tetris);
    return RenderTetris(tetris, renderer);
  }

  if (KeyUpThisFrame(input, Key::kSpace))
    MoveBlocksDown(tetris, 0);
  return RenderTetris(tetris, renderer);
}

}  // namespace tetris
