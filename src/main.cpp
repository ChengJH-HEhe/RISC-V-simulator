#include "config.hpp"
#include "memory.hpp"
#include "utility.hpp"

#include <iostream>

namespace CPU {
int pc_nxt, pc_pred;
int clk;
ALU alu;
ROB rob;
RS rs;
LSB lsb;
IQ iq;
ins ins;
regfile reg;
MEM Mem;
bool halt, reset;
inline void updateAll() {
  // TODO
  alu.update();
  rob.update();
  rs.update();
  lsb.update();
  iq.update();
}
inline void resetAll() {
  // TODO
  reset = false;
  pc_pred = pc_nxt;
  alu.reset();
  rob.reset();
  rs.reset();
  lsb.reset();
  iq.reset();
}
inline void issue() {
  if (!rob.cur.full() && !iq.cur.empty()) {
    auto cmd = iq.cur.front();
    iq.nxt.pop();
    cmd.robID = rob.cur.tailNum();
    // rob_node qj,qk -> regf
    robNode rbnode(cmd, false);
    if (cmd.rd != -1)
      reg.alu(cmd.rd, cmd.robID);
    insNode rsnode(cmd, reg);
    if (Ltype(cmd.type) || Stype(cmd.type)) {
      lsb.nxt.push(rsnode);
    } else {
      if (RItype(cmd.type))
        rsnode.Qk = -1, rsnode.Vk = cmd.imm;
      else if (!RBtype(cmd.type)) {
        rbnode.ready = true;
        switch (cmd.type) {
        case JAL:
          rbnode.value = cmd.pc + 4;
          break;
        case JALR:
          rbnode.ready = false;
          break;
        case AUIPC:
          rbnode.value = cmd.pc + cmd.imm;
          break;
        case LUI:
          rbnode.value = cmd.imm;
          break;
        default:
          assert(0);
        }
      }
      if (rsnode.Qj >= 0 && rob.cur[rsnode.Qj].ready)
        rsnode.Vj = rob.cur[rsnode.Qj].value, rsnode.Qj = -1;
      if (rsnode.Qk >= 0 && rob.cur[rsnode.Qk].ready)
        rsnode.Vk = rob.cur[rsnode.Qk].value, rsnode.Qk = -1;
      rs.nxt.insert(rsnode);
    }
    rob.nxt.push(rbnode);
  }
}

inline void execute() {
  rs.execute(&alu);
  lsb.execute(&reg, &rob, &Mem);
  alu.execute(&rob, &rs, &lsb, &reg);
  if (rob.execute(&reg, &iq, &lsb, &Mem, &reset))
    halt = true;
}
inline void process() {
  for (;;) {
    ++clk;
    if (reset)
      resetAll();
    updateAll();
    iq.fetch(clk, &Mem);
    issue();
    execute();
    if (reset) {
      resetAll();
      reset = false;
    }
    if (halt)
      break;
  }
  updateAll();
  std::cout << (reg.cur[10].value & 255u) << std::endl;
}
} // namespace CPU

#include <unistd.h>
#include <limits.h>

std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(result, count);
    } else {
        return std::string();
    }
}

int main() { 
  freopen("array_test1.data","r",stdin); 
  CPU::Mem.loadInput();
  CPU::process();
}