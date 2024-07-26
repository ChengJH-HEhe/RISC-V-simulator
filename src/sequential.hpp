#ifndef SEQUENTIAL_HPP
#define SEQUENTIAL_HPP

#include "config.hpp"
#include "memory.hpp"


struct CPU{
  unsigned int pc_in, pc_out;
  ins ins;
  bool halt;
}toyCPU;

void updateAll(){
  toyCPU.pc_out = toyCPU.pc_in;
}

void fetch(){
  unsigned code = loadIns(toyCPU.pc_in);
  toyCPU.ins = decodeIns(code, toyCPU.pc_in);
  if(code == 0x0ff00513) {
    toyCPU.halt = 1;

    return;
  }
}

void execute(){
  switch(toyCPU.ins.type) {
    case NIL: return;
    
    default: return;
  }
}

#endif