// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "disassembler.h"

#include <deque>

#include "gameboy.h"

namespace rothko {
namespace emulator {

namespace {

// Additional information about instructions that interrupt the normal flow of a program.
// Some instructions change what instructions should be considered.
//
// NOTE: The operand will be tried to be used to determine a next disassemble target.
//       A one byte operand (r8) will be used for a relative jump. A two-byte one (a16) determines
//       that it's an absolute jump.
struct ConditionalInstruction {
  // Whether the disassembler should consider the directly next instruction as a valid one to
  // disassemble.
  bool next_inst_valid = false;

  // An additional instruction that this should be considered for disassembly. -1 means none.
  static constexpr uint64_t kNoAdditionalTarget = (uint16_t)-1;
  uint16_t additional_target = kNoAdditionalTarget;
};


std::map<uint16_t, ConditionalInstruction> CreateConditionalInstructions() {
  std::map<uint16_t, ConditionalInstruction> instructions = {};
  instructions[0x18] = {false};           // JR r8
  instructions[0x20] = {true};            // JR NZ, r8
  instructions[0x28] = {true};            // JR Z, r8
  instructions[0x30] = {true};            // JR NC, r8
  instructions[0x38] = {true};            // JR C, r8
  instructions[0xc0] = {true};            // RET NZ
  instructions[0xc2] = {true};            // JP NZ, a16
  instructions[0xc3] = {false};           // JP a16
  instructions[0xc4] = {true};            // CALL NZ, a16
  instructions[0xc7] = {false, 0x000};    // RST 0x00
  instructions[0xc8] = {true};            // RET Z
  instructions[0xc9] = {false};           // RET
  instructions[0xca] = {true};            // JP Z, a16
  instructions[0xcc] = {true};            // CALL Z, a16
  instructions[0xcd] = {false};           // CALL a16
  instructions[0xcf] = {false, 0x0008};   // RST 0x08
  instructions[0xd0] = {true};            // RET NC
  instructions[0xd2] = {true};            // JP NC, a16
  instructions[0xd4] = {true};            // CALL NC, a16
  instructions[0xd7] = {false, 0x0010};   // RST 0x10
  instructions[0xd8] = {true};            // RET C
  instructions[0xd9] = {false};           // RETI
  instructions[0xda] = {true};            // JP C, a16
  instructions[0xdc] = {true};            // CALL C, a16
  instructions[0xdf] = {false, 0x0018};   // RST 0x18
  instructions[0xe7] = {false, 0x0020};   // RST 0x20
  instructions[0xe9] = {false};           // JP (HL)
  instructions[0xef] = {false, 0x0028};   // RST 0x28
  instructions[0xf7] = {false, 0x0030};   // RST 0x30
  instructions[0xff] = {false, 0x0038};   // RST 0x38

  return instructions;
}


// Map to know if we already touched that instructions.
std::unique_ptr<uint8_t[]> gTouchedMap = std::make_unique<uint8_t[]>(0x10000);
const std::map<uint16_t, ConditionalInstruction> kConditionalInstructions =
    CreateConditionalInstructions();

void DisassembleConditionalInstructions(DisassembledInstruction* dis_inst,
                                        std::deque<uint16_t>* pending_queue,
                                        uint16_t address) {
  // |next_address| was already verified.
  uint16_t next_address = address + dis_inst->instruction.length;

  // We already verified that the conditional instruction is there.
  auto it = kConditionalInstructions.find(address);
  if (it->second.next_inst_valid)
    pending_queue->push_back(next_address);

  // See if this is a relative/absolute jump.
  if (dis_inst->instruction.length == 2) {
    // Relative jump. The one-byte offset is signed.
    int8_t offset = (int8_t)dis_inst->instruction.operands[0];
    uint16_t rel_address = address + offset;
    if (gTouchedMap[rel_address] == 0){
      gTouchedMap[rel_address] = 1;
      pending_queue->push_back(rel_address);
    }
  } else if(dis_inst->instruction.length == 3) {
    // Absolute jump.
    uint16_t abs_address = dis_inst->instruction.operand;
    if (gTouchedMap[abs_address] == 0) {
      gTouchedMap[abs_address] = 1;
      pending_queue->push_back(abs_address);
    }
  }

  uint16_t additional_target = it->second.additional_target;
  // If we have and additional target to check (this is an unconditional known jump, or RST), we
  // add it if we haven't already.
  if (additional_target != ConditionalInstruction::kNoAdditionalTarget) {
    if (gTouchedMap[additional_target] == 0) {
      gTouchedMap[additional_target] = 1;
      pending_queue->push_back(additional_target);
    }
  }
}

bool DisassembleInstruction(const Gameboy& gameboy, DisassembledInstruction* dis_inst,
                            std::deque<uint16_t>* pending_queue, uint16_t address) {

  auto touched_map = std::make_unique<uint8_t[]>(0x10000);

  const uint8_t* base_ptr = (const uint8_t*)&gameboy.memory;

  if (!FetchAndDecode(&dis_inst->instruction, base_ptr + address))
    return false;

  dis_inst->address = address;

  // See if this address or any of the next have been already touched.
  uint16_t next_address = address + dis_inst->instruction.length;
  ASSERT(next_address > address);
  for (uint16_t a = address; a <= next_address; a++) {
    if (touched_map[a] == 1) {
      NOT_REACHED_MSG("Instruction 0x%x already disassembled.", a);
      return false;
    }

    touched_map[a] = 1;
  }

  // TODO: Obtain name and description of instructions.

  auto it = kConditionalInstructions.find(address);
  if (it != kConditionalInstructions.end()) {
    DisassembleConditionalInstructions(dis_inst, pending_queue, address);
    return true;
  }

  // We add the new instruction to the queue.
  pending_queue->push_back(next_address);
  return true;
}

}  // namespace


void Disassemble(const Gameboy& gameboy, Disassembler* disassembler, uint16_t entry_point) {
  // Reset the touched map.
  memset(gTouchedMap.get(), 0, 0x10000);

  // Addresses that are still pending to be disassembled.
  std::deque<uint16_t> pending_queue;

  auto& instructions = disassembler->instructions;
  instructions.clear();

  // Add always add the gameboy begin point. And then also add the |entry_point|.
  pending_queue.push_back(0x100);
  gTouchedMap[0x100] = 1;
  if (entry_point != 0x100) {
    pending_queue.push_back(entry_point);
    gTouchedMap[entry_point] = 1;
  }

  while (!pending_queue.empty()) {
    // Take the first address of the queue.
    uint64_t address = pending_queue.front();
    pending_queue.pop_front();

    // If we cannot take 3 bytes of context, we do not disassemble.
    if (address == 0xffff - 1)
      continue;

    // If we already disassembled this instruction, we don't continue.
    if (instructions.find(address) != instructions.end())
      continue;

    DisassembledInstruction dis_inst = {};
    if (!DisassembleInstruction(gameboy, &dis_inst, &pending_queue, address))
      continue;

    // Finally we add it to the disassembled instructions.
    disassembler->instructions[address] = std::move(dis_inst);
  }
}


}  // namespace emulator
}  // namespace rothko
