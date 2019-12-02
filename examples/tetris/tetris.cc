// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <rothko/game.h>
#include <rothko/graphics/commands.h>
#include <rothko/input/input.h>
#include <rothko/utils/defer.h>
#include <rothko/widgets/lines.h>

using namespace rothko;

namespace tetris {

struct TetrisImpl {
  LineManager lines;
};

namespace {

struct Shape {
  uint8_t type = 0;
  Int2 offsets[4];
  Int2 bounds = {};
};

Shape CreateShape(uint32_t type, std::vector<Int2> elems) {
  Shape shape = {};
  shape.type = type;
  uint32_t i = 0;
  for (Int2 elem : elems) {
    shape.offsets[i++] = elem;
    shape.bounds.x = Min(shape.bounds.x, elem.x);
    shape.bounds.y = Max(shape.bounds.y, elem.x);
  }

  return shape;
}

struct ShapeGroup {
  ShapeGroup() = default;

  Shape shapes[4] = {};  // Shapes have at most 4 rotations.
  uint8_t rotation_count = 0;
  uint8_t valid = 0;
};

std::pair<uint8_t, const Shape&>
GetNextRotation(const ShapeGroup& shape_group, uint8_t current_rotation) {
  uint8_t next_index = current_rotation + 1;
  if (next_index > shape_group.rotation_count - 1)
    next_index = 0;

  return {next_index, shape_group.shapes[next_index]};
}

inline std::vector<ShapeGroup> CreateShapeGroups() {
  std::vector<ShapeGroup> groups;
  ShapeGroup group = {};
  groups.push_back(group);   // Empty one.

  // clang-format off

  // Square.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {Int2{0, 0}, Int2{0, 1}, Int2{1, 0}, Int2{1, 1}});
  group.rotation_count = 1;
  groups.push_back(std::move(group));

  // S.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {{0, 0}, {-1,  0}, { 0,  1}, {1,  1}});
  group.shapes[1] = CreateShape(groups.size(), {{0, 0}, { 0,  1}, { 1,  0}, {1, -1}});
  group.shapes[2] = CreateShape(groups.size(), {{0, 0}, { 0, -1}, {-1, -1}, {1,  0}});
  group.shapes[3] = CreateShape(groups.size(), {{0, 0}, {-1,  0}, {-1,  1}, {0, -1}});
  group.rotation_count = 4;
  groups.push_back(std::move(group));

  // Z.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {{}, {-1,  1}, { 0,  1}, {1,  0}});
  group.shapes[1] = CreateShape(groups.size(), {{}, { 0, -1}, { 1,  0}, {1,  1}});
  group.shapes[2] = CreateShape(groups.size(), {{}, {-1,  0}, { 0, -1}, {1, -1}});
  group.shapes[3] = CreateShape(groups.size(), {{}, {-1, -1}, {-1,  0}, {0,  1}});
  group.rotation_count = 4;
  groups.push_back(std::move(group));

  // T.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {{}, {-1,  0}, { 0,  1}, {1,  0}});
  group.shapes[1] = CreateShape(groups.size(), {{}, { 0,  1}, { 1,  0}, {0, -1}});
  group.shapes[2] = CreateShape(groups.size(), {{}, {-1,  0}, { 0, -1}, {1,  0}});
  group.shapes[3] = CreateShape(groups.size(), {{}, { 0, -1}, {-1,  0}, {0,  1}});
  group.rotation_count = 4;
  groups.push_back(std::move(group));

  // L.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {{}, {-1,  0}, { 1,  0}, { 1,  1}});
  group.shapes[1] = CreateShape(groups.size(), {{}, { 0,  1}, { 0, -1}, { 1, -1}});
  group.shapes[2] = CreateShape(groups.size(), {{}, { 1,  0}, {-1,  0}, {-1, -1}});
  group.shapes[3] = CreateShape(groups.size(), {{}, { 0, -1}, { 0,  1}, {-1,  1}});
  group.rotation_count = 4;
  groups.push_back(std::move(group));

  // Reserve-L.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {{}, {-1,  1}, {-1,  0}, { 1,  0}});
  group.shapes[1] = CreateShape(groups.size(), {{}, { 1,  1}, { 0,  1}, { 0, -1}});
  group.shapes[2] = CreateShape(groups.size(), {{}, { 1, -1}, { 1,  0}, {-1,  0}});
  group.shapes[3] = CreateShape(groups.size(), {{}, {-1, -1}, { 0, -1}, { 0,  1}});
  group.rotation_count = 4;
  groups.push_back(std::move(group));

  // Line.
  group = {}; group.valid = 1;
  group.shapes[0] = CreateShape(groups.size(), {{}, {-1,  0}, { 1,  0}, {2,  0}});
  group.shapes[1] = CreateShape(groups.size(), {{}, { 0,  1}, { 0, -1}, {0, -2}});
  group.shapes[2] = CreateShape(groups.size(), {{}, {-2,  0}, {-1,  0}, {1,  0}});
  group.shapes[3] = CreateShape(groups.size(), {{}, { 0, -1}, { 0,  1}, {0,  2}});
  group.rotation_count = 4;
  groups.push_back(std::move(group));

  // clang-format off;

  ASSERT(std::size(groups) < kCurrentShapeMask);
  return groups;
}

const std::vector<ShapeGroup> kShapes = CreateShapeGroups();

inline uint32_t GetShapeIndex() { return Random(1, std::size(kShapes) - 1); }

}  // namespace

// Init --------------------------------------------------------------------------------------------

namespace {

// Upside down!
// clang-format off
constexpr uint32_t kTestBoard[kTetrisSizeX * kTetrisSizeY] = {
  /*      0  1  2  3  4  5  6  7  8  9 */
  /*  0 */ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  1 */ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  2 */ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  /*  3 */ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
  /*  4 */ 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
  /*  5 */ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
  /*  6 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  7 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 11 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 12 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 13 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 14 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 15 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 17 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 18 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 19 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
// clang-format on

} // namespace

std::unique_ptr<Tetris> InitTetris(Renderer* renderer) {
  auto tetris = std::make_unique<Tetris>();

  tetris->impl = new TetrisImpl();
  if (!Init(&tetris->impl->lines, renderer, "tetris-lines"))
    return nullptr;

  SetNextShape(&tetris->flags, GetShapeIndex());

  for (uint32_t i = 0; i < std::size(kTestBoard); i++) {
    tetris->board[i] = kTestBoard[i];
  }

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
  const ShapeGroup& shape_group = kShapes[shape_index];
  const Shape& shape = shape_group.shapes[GetCurrentRotation(tetris.flags)];

  return {shape, tetris.shape_pos};
}

void ClearShapeBlocks(Tetris* tetris, const Shape& shape, Int2 shape_pos) {
  for (Int2 offset : shape.offsets) {
    Int2 pos = shape_pos + offset;
    SetBlock(tetris, pos.x, pos.y, 0);
  }
}

void SetShapeBlocks(Tetris* tetris, const Shape& shape, Int2 shape_pos) {
  for (uint32_t i = 0; i < std::size(shape.offsets); i++) {
    Int2 pos = shape_pos + shape.offsets[i];

    uint32_t block = 1;
    SetBlockType(&block, shape.type);
    if (shape.offsets[i] == Int2::Zero())
      SetShapeOrigin(&block);
    SetBlock(tetris, pos.x, pos.y, block);
  }
}

void ClearCurrentShapeState(Tetris* tetris) {
  SetCurrentShape(&tetris->flags, 0);
  SetCurrentRotation(&tetris->flags, 0);
  tetris->shape_pos = {};
}

// Returns true if a new shape was created.
enum class NewShapeResult {
  kNone,
  kPass,
  kCreated,
};
NewShapeResult CheckForNewShape(Tetris* tetris, const Input& input) {
  if (GetCurrentShape(tetris->flags))
    return NewShapeResult::kNone;

  uint32_t shape_index = GetNextShape(tetris->flags);
  ASSERT(shape_index);

  if (KeyUpThisFrame(input, Key::kSpace)) {
    // We want a different next shape.
    uint32_t current_shape = GetNextShape(tetris->flags);
    uint32_t next_shape = GetShapeIndex();
    while (next_shape == current_shape) {
      next_shape = GetShapeIndex();
    }

    SetNextShape(&tetris->flags, next_shape);
    return NewShapeResult::kPass;
  }

  // Choose a new shape randomly.
  const ShapeGroup& shape_group = kShapes[shape_index];
  const Shape& shape = shape_group.shapes[0];

  int base = (int)Key::k1;
  int place_index = -1;
  for (int i = 1; i <= 9; i++) {
    int index = i - 1;
    Key key = (Key)(base + index);
    if (!KeyUpThisFrame(input, key))
      continue;
    place_index = index;
  }

  // |place_index| < 0  means that no input was pressed.
  if (place_index < 0)
    return NewShapeResult::kPass;

  if (place_index + shape.bounds.min < 0 ||
      (uint32_t)place_index >= kTetrisSizeX - shape.bounds.max) {
    WARNING(App, "Could not create shape. Place index: %d. Bounds: %s",
            place_index, ToString(shape.bounds).c_str());
    return NewShapeResult::kPass;
  }

  SetCurrentShape(&tetris->flags, shape_index);

  // Choose random place to place it.
  /* int place_index = Random(0, kTetrisSizeX - shape.bounds.y); */
  tetris->shape_pos.x = place_index;
  tetris->shape_pos.y = kTetrisSizeY;

  // Mark the new places with this shape.
  SetShapeBlocks(tetris, shape, tetris->shape_pos);

  return NewShapeResult::kCreated;
}

// Check that all the shapes don't clash with an already existing, non-shape block.
bool VerifyShapeCollision(const Tetris& tetris, const Shape& shape, Int2 shape_pos, Int2 offset) {
  for (Int2 shape_offset : shape.offsets) {
    Int2 pos = shape_pos + shape_offset + offset;

    // Board bounds check.
    if (pos.x < 0 || pos.x >= (int)kTetrisSizeX || pos.y < 0)
      return false;

    uint32_t block = GetBlock(tetris, pos.x, pos.y);
    // If there is no block there, it's a valid move.
    if (!GetBlockPresence(block))
      continue;

    // We need to check that it is effectively a non-shape block. That is BlockType == 0.
    if (!GetBlockType(block))
      return false;
  }

  return true;
}

void MoveShapeBlocks(Tetris* tetris, const Shape& shape, Int2 shape_pos, Int2 offset) {
  ClearShapeBlocks(tetris, shape, shape_pos);
  SetShapeBlocks(tetris, shape, shape_pos + offset);

  // Now move the saved values.
  tetris->shape_pos = shape_pos + offset;
}

void MoveBlocksDown(Tetris* tetris, uint32_t row) {
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
  ClearCurrentShapeState(tetris);

  // Change the next shape.
  SetNextShape(&tetris->flags, GetShapeIndex());
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

enum class MoveResult {
  kNone,
  kMove,
  kCollision,
};

MoveResult MoveShapeDown(Tetris* tetris) {
  const auto& [shape, shape_pos] = GetShape(*tetris);
  if (!VerifyShapeCollision(*tetris, shape, tetris->shape_pos, {0, -1})) {
    DoShapeCollision(tetris);
    CheckForCompleteRows(tetris);
    return MoveResult::kCollision;
  }

  MoveShapeBlocks(tetris, shape, tetris->shape_pos, {0, -1});
  return MoveResult::kMove;
}

MoveResult RotateShape(Tetris* tetris) {
  const ShapeGroup& shape_group = kShapes[GetCurrentShape(tetris->flags)];
  const auto& [rot_index, rot_shape] = GetNextRotation(shape_group,
                                                       GetCurrentRotation(tetris->flags));

  // Check if the rotated shape will collide with something.
  if (!VerifyShapeCollision(*tetris, rot_shape, tetris->shape_pos, {}))
    return MoveResult::kCollision;

  // Move the shape.
  const auto& [shape, shape_pos] = GetShape(*tetris);
  ClearShapeBlocks(tetris, shape, shape_pos);
  SetShapeBlocks(tetris, rot_shape, shape_pos);

  // Update the rotation index.
  SetCurrentRotation(&tetris->flags, rot_index);
  return MoveResult::kMove;
}

MoveResult MoveShape(Tetris* tetris, const Game& game) {
  const auto& [shape, shape_pos] = GetShape(*tetris);

  if (game.time.seconds > tetris->last_move_down + tetris->time_move_down) {
    tetris->last_move_down = game.time.seconds;
    return MoveShapeDown(tetris);
  }

  int offsetX = 0;
  if (KeyDown(game.input, Key::kLeft)) {
    offsetX = -1;
  } else if (KeyDown(game.input, Key::kRight)) {
    offsetX = 1;
  } else if (KeyDown(game.input, Key::kDown)) {
    if (game.time.seconds > tetris->last_press_down + tetris->time_press_down) {
      tetris->last_press_down = game.time.seconds;
      tetris->last_move_down = game.time.seconds;
      return MoveShapeDown(tetris);
    }
  } else if (KeyDownThisFrame(game.input, Key::kUp)) {
    return RotateShape(tetris);
  }

  // No key pressed.
  if (offsetX == 0) {
    // Reset any move timing.
    if (KeyUpThisFrame(game.input, Key::kLeft) || KeyUpThisFrame(game.input, Key::kRight))
      tetris->last_move_side = 0;
    return MoveResult::kNone;
  }

  // We only move sideways when it's time.
  if (game.time.seconds <= tetris->last_move_side + tetris->time_move_side)
    return MoveResult::kNone;
  tetris->last_move_side = game.time.seconds;

  if (!VerifyShapeCollision(*tetris, shape, shape_pos, {offsetX, 0}))
    return MoveResult::kCollision;

  if (!VerifyShapeCollision(*tetris, shape, tetris->shape_pos, {offsetX, 0}))
    return MoveResult::kNone;

  MoveShapeBlocks(tetris, shape, tetris->shape_pos, {offsetX, 0});
  return MoveResult::kMove;
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

      Color color = Color::Red();

      if (GetBlockType(block))
        color = GetShapeOrigin(block) ? Color::Orange() : Color::Green();

      PushCube(&impl->lines,
               Vec3(x + kBorder, y + kBorder, 0),
               Vec3(x + 1 - kBorder, y + 1 - kBorder, 1),
               color);
    }
  }

  // Next Shape.
  const ShapeGroup& next_shape_group = kShapes[GetNextShape(tetris->flags)];
  const Shape& next_shape = next_shape_group.shapes[0];

  Int2 o = {-4, 6};
  for (auto& offset : next_shape.offsets) {
    Vec3 base = {(float)(o.x + offset.x), (float)(o.y + offset.y), 0};

    PushCube(&impl->lines, base, base + Vec3{1, 1, 1}, Color::Yellow());
  }

  if (!Stage(&impl->lines, renderer))
    return {};
  return GetRenderCommand(impl->lines);
}

}  // namespace

RenderCommand Update(Tetris* tetris, Game* game) {
  ASSERT(tetris->impl);

  // We see if we need to create a new shape.
  if (NewShapeResult result = CheckForNewShape(tetris, game->input);
      result == NewShapeResult::kPass || result == NewShapeResult::kCreated) {
    return RenderTetris(tetris, game->renderer.get());
  }

  MoveShape(tetris, *game);
  /*     return RenderTetris(tetris, game->renderer.get()); */

  /* if (KeyUpThisFrame(game->input, Key::kSpace)) */
  /*   MoveBlocksDown(tetris, 0); */
  return RenderTetris(tetris, game->renderer.get());
}

}  // namespace tetris
