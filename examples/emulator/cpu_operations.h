// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <utility>

namespace rothko {
namespace emulator {

struct CPU;

/* uint8_t TestBit(CPU*, uint8_t word, int bit); */
/* uint8_t SetBit(CPU*, uint8_t word, int bit); */
/* uint8_t ClearBit(CPU*, uint8_t word, int bit); */
/* uint8_t RotateLeft(CPU*, uint8_t value, int count = 1); */
/* uint8_t RotateRight(CPU*, uint8_t value, int count = 1); */
/* std::pair<uint8_t, uint8_t> RotateLeftThroughCarry(CPU*, uint8_t value, int count = 1, int carry = 0); */
/* std::pair<uint8_t, uint8_t> RotateLeftAndCarry(CPU*, uint8_t value, int count = 1); */
/* std::pair<uint8_t, uint8_t> RotateRightThroughCarry(CPU*, uint8_t value, int count = 1, int carry = 0); */
/* std::pair<uint8_t, uint8_t> RotateRightAndCarry(CPU*, uint16_t value, int count = 1); */
/* std::pair<uint8_t, uint8_t> ShiftLeft(CPU*, uint8_t value, int count = 1); */
/* std::pair<uint8_t, uint8_t> ShiftRightLogic(CPU*, uint8_t value, int count = 1); */
/* std::pair<uint8_t, uint8_t> ShiftRightArithmetic(CPU*, uint16_t value, int count = 1); */
/* uint8_t SwapNibbles(CPU*, uint8_t value); */
/* void SignedAdd(CPU*, uint16_t target, int8_t operand); */
/* void SBC(CPU*, uint8_t substractee, uint8_t substractor, uint8_t extraSub); */
/* int Interpolate(CPU*, int minValue, int maxValue, int minIndex, int maxIndex, int value); */

// Common operations that occur on many instructions, which register is used being the only
// difference between them. Each instructions has a comment about the flags affected.
// * = Variable result, 0 = Always cleared, 1 = Always set, / = Not affected.


// Z=* N=0 H=1 C=/
void TestBit(CPU*, uint8_t word, int bit);

// Z=/ N=/ H=/ C=/
uint8_t SetBit(uint8_t* target, int bit);

// Z=/ N=/ H=/ C=/
uint8_t ClearBit(uint8_t* target, int bit);

// Z=* N=0 H=0 C=*
void RotateLeftThroughCarry(CPU*, uint8_t* target, int count = 1, int carry = 0);

// Z=* N=0 H=0 C=*
void RotateLeftAndCarry(CPU*, uint8_t* target, int count = 1);

// Z=* N=0 H=0 C=*
void RotateRightThroughCarry(CPU*, uint8_t* target, int count = 1, int carry = 0);

// Z=* N=0 H=0 C=*
void RotateRightAndCarry(CPU*, uint8_t* target, int count = 1);

// Z=* N=0 H=0 C=*
void ShiftLeft(CPU*, uint8_t* target, int count = 1);

// Z=* N=0 H=0 C=*
void ShiftRightLogic(CPU*, uint8_t* target, int count = 1);

// Z=* N=0 H=0 C=*
void ShiftRightArithmetic(CPU*, uint8_t* target, int count = 1);

// Z=* N=0 H=0 C=0
uint8_t SwapNibbles(CPU* cpu, uint8_t* target);

// Z=* N=1 H=* C=*
void SBC(CPU*, uint8_t* substractee, uint8_t substractor, uint8_t extraSub);

}  // namespace emulator
}  // namespace rothko
