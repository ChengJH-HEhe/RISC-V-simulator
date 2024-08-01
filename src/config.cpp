#include "config.hpp"
#include <iostream>
#include <istream>
std::string typee[]={
  "LUI",
  "AUIPC", // U 0,1
  "JAL",   // J 2
  "JALR",  // I 3
  "BEQ",
  "BGE",
  "BGEU",
  "BLT",
  "BLTU",
  "BNE", // B 4 9
  "ADD",
  "SUB",
  "AND",
  "OR",
  "XOR", // R 10, 14
  "SLL",
  "SRL",
  "SRA",
  "SLT",
  "SLTU", // R 15, 19
  "ADDI",
  "ANDI",
  "ORI",
  "XORI", // I 20, 23
  "SLLI",
  "SRLI",
  "SRAI", // I* 24, 26
  "SLTI",
  "SLTIU", // I 27, 28
  "LB",
  "LBU",
  "LH",
  "LHU",
  "LW", // L 29, 33
  "SB",
  "SH",
  "SW", // S 34, 36
  "LI"  // ret
};
void CPU::ins::output(std::ostream& ci) {
  if(type >= 0 && type <= 37)
  ci << "pc=" << pc << " type=" << typee[type] << " rd=" << rd
            << " rs1=" << rs1 << " rs2=" << rs2 << " imm=" << imm << std::endl;
  else {
    std::cout << type << " " ;
    assert(0);
  }
}
void CPU::insNode::output(){
  std::cerr << "pc=" << pc << " type=" << typee[op] << " rd=" << rd
            << " Vj={" << Qj << " "<< Vj << "}"  << " Vk={" << Qk << " "<< Vk << "}"<< " imm=" << imm << std::endl;
}

bool CPU::RBtype(Instype _) {
  return (BEQ <= _ && _ <= BNE) || (ADD <= _ && _ <= SLTIU);
}
bool CPU::Rtype(Instype _) { return (ADD <= _ && _ <= SLTIU); }
bool CPU::RItype(Instype _) { return (ADDI <= _ && _ <= SLTIU); }
bool CPU::Btype(Instype _) { return (BEQ <= _ && _ <= BNE); }
bool CPU::Ltype(Instype _) { return (LB <= _ && _ <= LW); }
bool CPU::Stype(Instype _) { return (SB <= _ && _ <= SW); }