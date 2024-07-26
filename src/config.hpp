#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "memory.hpp"
#include <iostream>

enum Instype {
  NIL = -1,
  LUI,
  AUIPC, // U 0,1
  JAL,   // J 2
  JALR,  // I 3
  BEQ,
  BGE,
  BGEU,
  BLT,
  BLTU,
  BNE, // B 4 9
  ADD,
  SUB,
  AND,
  OR,
  XOR, // R 10, 14
  SLL,
  SRL,
  SRA,
  SLT,
  SLTU, // R 15, 19
  ADDI,
  ANDI,
  ORI,
  XORI, // I 20, 23
  SLLI,
  SRLI,
  SRAI, // I* 24, 26
  SLTI,
  SLTIU, // I 27, 28
  LB,
  LBU,
  LH,
  LHU,
  LW, // L 29, 33
  SB,
  SH,
  SW, // S 34, 36
  LI  // ret
};

const int Rop = 0b0110011, Iop = 0b0010011, Sop = 0b0100011, Bop = 0b1100011;

struct ins {
  Instype type;
  unsigned int rs2, rs1, rd;
  int imm;
};

unsigned int loadIns(unsigned int pc) {
  unsigned int res = 0;
  for (int i = 0; i < 4; ++i) {
    res |= (mem[pc++] << (i * 8));
    // std::cout << mem[pc] << " ";
  }
  // std::cout << res << std::endl;
  return res;
}
// load immediate & determine the type
void R(ins &res, int funct3, int funct7, unsigned int code) {
  switch (funct3) {
  case 0b000: {
    if (funct7 == 0b0000000)
      res.type = ADD;
    else if (funct7 == 0b0100000)
      res.type = SUB;
  } break;
  case 0b111:
    res.type = AND;
    break;
  case 0b110:
    res.type = OR;
    break;
  case 0b100:
    res.type = XOR;
    break;
  case 0b001:
    res.type = SLL;
    break;
  case 0b101: {
    if (funct7 == 0b0000000)
      res.type = SRL;
    else
      res.type = SRA;
    break;
  }
  case 0b010:
    res.type = SLT;
    break;
  case 0b011:
    res.type = SLTU;
    break;
  }
}
void I(ins &res, int funct3, int funct7, unsigned int code) {
  switch (funct3) {
  case 0b000: {
      res.type = ADDI;
  } break;
  case 0b111:
    res.type = ANDI;
    break;
  case 0b110:
    res.type = ORI;
    break;
  case 0b100:
    res.type = XORI;
    break;
  case 0b001:
    res.type = SLLI;
    break;
  case 0b101: {
    if (funct7 == 0b0000000)
      res.type = SRLI;
    else
      res.type = SRAI;
    break;
  }
  case 0b010:
    res.type = SLTI;
    break;
  case 0b011:
    res.type = SLTIU;
    break;
  }
}

void S(ins &res, int funct3, int funct7, unsigned int code) {
  
}

void B(ins &res, int funct3, int funct7, unsigned int code) {}

ins decodeIns(unsigned int code, unsigned int pc) {
  ins res;
  if (code == 0x0ff00513) {
    res.type = LI;
    return res;
  }
  int opcode = code & 0x7F, funct3 = (code >> 12) & 7,
      funct7 = (code >> 25 & 0x7F);
  res.rd = code >> 7 & 0x1F;
  res.rs1 = code >> 15 & 0x1F;
  res.rs2 = code >> 20 & 0x1F;

  switch (opcode) {
  case Rop:
    R(res, funct3, funct7, code);
    break;
  case Iop:
    I(res, funct3, funct7, code);
    break;
  case Bop:
    B(res, funct3, funct7, code);
    break;
  case Sop:
    S(res, funct3, funct7, code);
    break;
  default:
    if (opcode == 0b1101111) { // JAL
      res.type = JAL;

    } else if (opcode == 0b1100111) {
      res.type = JALR;

    } else if (opcode == 0b0010111) {
      res.type = AUIPC;

    } else if (opcode == 0b0110111) {
      res.type = LUI;
    }
  }
  return res;
}

#endif