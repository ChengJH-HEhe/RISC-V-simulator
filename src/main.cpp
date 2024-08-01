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
regfile reg;
MEM Mem;
pc_predictor BP;
bool halt, reset;
int total, ac;
inline void updateAll() {
  // TODO
  alu.update();
  rob.update();
  reg.update();
  rs.update();
  lsb.update();
  iq.update();
  BP.update();
}
inline void output(){
  std::cerr << "----------------REG------------------" << std::endl;
  reg.output();
  std::cerr << "----------------RS------------------" << std::endl;
  rs.output();
  std::cerr << "----------------LSB------------------" << std::endl;
  lsb.output();
  std::cerr << "----------------ROB------------------" << std::endl;
  rob.output();
}
inline void resetAll() {
  // TODO
  reset = false;
  pc_pred = pc_nxt;
  //std::cerr << "reset pc_nxt " << pc_nxt << std::endl;
  alu.reset();
  rob.reset();
  rs.reset();
  lsb.reset();
  iq.reset();
  reg.reset();
}
inline void issue() {
  if (!rob.cur.full() && !iq.cur.empty()) {
    auto cmd = iq.cur.front();
    iq.nxt.pop();
    cmd.robID = rob.cur.tailNum();
    // rob_node qj,qk -> regf
    if(cmd.type == LI) {
      robNode rbnode(cmd, true);
      rob.nxt.push(rbnode);
      return;
    }
    robNode rbnode(cmd, false);

    // std::cerr << "ISSUE " << rob.cur.tailNum()<< " ";
    // cmd.output();
    
    if (cmd.rd != -1)
      assert(cmd.rd>=0), reg.alu(cmd.rd, cmd.robID);
    insNode rsnode(cmd, reg);
    if (!cmd.flg1 && rsnode.Qj >= 0 && rob.cur[rsnode.Qj].ready)
      rsnode.Vj = rob.cur[rsnode.Qj].value, rsnode.Qj = -1;
    if (!cmd.flg2 && rsnode.Qk >= 0 && rob.cur[rsnode.Qk].ready)
      rsnode.Vk = rob.cur[rsnode.Qk].value, rsnode.Qk = -1;
    //rsnode.output();
    //  lsb or rsnode
    if (Ltype(cmd.type) || Stype(cmd.type)) {
      lsb.nxt.push(rsnode);
    } else {
      if (RItype(cmd.type))
        rsnode.Qk = -1, rsnode.Vk = cmd.imm, rs.nxt.insert(rsnode);
      else if (!RBtype(cmd.type)) {
        rbnode.ready = true;
        switch (cmd.type) {
        case JAL:
          rbnode.value = cmd.pc + 4;
          break;
        case JALR:
          rs.nxt.insert(rsnode);
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
      } else {
        rs.nxt.insert(rsnode);
      }
    }
    rob.nxt.push(rbnode);
  }
}

inline void execute() {
  rs.execute(&alu);
  alu.execute(&rob, &rs, &lsb, &reg);
  if (rob.execute(&reg, &iq, &rs, &lsb, &Mem, &reset))
    halt = true;
  lsb.execute(&reg, &rob, &rs, &Mem);
}
inline void process() {
  for (;;) {
    ++clk;
    updateAll();
    if (reset)
      resetAll();
    //output();
    iq.fetch(clk, &Mem);
    execute();
    if (halt)
      break;
    issue();
  }
  updateAll();
  std::cout << (reg.cur[10].value & 255u) << std::endl;
}
} // namespace CPU

int main() { 
  //freopen("./src/array_test2.data","r",stdin); 
  //freopen("out.txt","w", stdout);
  CPU::Mem.loadInput();
  CPU::process();
}