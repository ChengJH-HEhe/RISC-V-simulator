#include "utility.hpp"
#include "config.hpp"

namespace CPU {
extern int pc_nxt, pc_pred;
pc_predictor BP;
void ALU::execute(ROB *rob, RS *rs, LSB *lsb, regfile *reg) {
  if (in.op == NIL)
    return;
  int result;
  if (RBtype(in.op)) {
    if (Rtype(in.op)) {
      result = R_calc(in);
    } else if (Btype(in.op)) {
      result = B_calc(in);
    } else
      assert(0);
  } else {
    switch (in.op) {
    case JALR: {
      result = in.pc + 4;
      // 特判
      rob->nxt[in.robID].pcDest = in.Vj + in.Vk; rob->nxt[in.robID].ready = true;
      std::cout << "JALR ready ";  rob->nxt[in.robID].cmd.output();
    } break;
    default:
      assert(0);
    }
  }
  rob->alu(result, in.robID); // execute is_busy = false;
  rs->alu(result, in.robID);  // rd = rs1 + rs2 何时标记 ？writeback
                              // 但未commit的时候 如果发现当前
  lsb->alu(result, in.robID);
}

void ROB::alu(int value, int id) {
  nxt[id].value = value;
  nxt[id].ready = true;
}
void ROB::commit(robNode rb, regfile* reg) {
  reg->rob(rb.cmd.rd, rb.value, rb.cmd.robID);
  pc_nxt = rb.cmd.pc + 4;
}
bool ROB::execute(regfile *reg, IQ *iq, LSB* lsb, MEM* Mem, bool *reset) {
  if (!cur.empty() && cur.front().ready) {
    if(ticker_cur) {
      if(ticker_cur == 3) {
        // load instruction
        auto node = lsb->cur.front();
        if(Stype(node.op)) {
          if (node.op == SB)
            Mem->write(node.Vj + node.imm, 1, node.Vk);
          else if (node.op == SH)
            Mem->write(node.Vj + node.imm, 2, node.Vk);
          else if (node.op == SW)
            Mem->write(node.Vj + node.imm, 4, node.Vk);
        } else {
          if (!Ltype(node.op))
            assert(0);
          if (node.op == LB)
            nxt[node.robID].value = Mem->read(node.Vj, 1);
          else if (node.op == LH)
            nxt[node.robID].value = Mem->read(node.Vj, 2);
          else if (node.op == LW)
            nxt[node.robID].value = Mem->read(node.Vj, 4);
          reg->chQ(node.robID, -1), reg->chV(node.robID, nxt[node.robID].value);
        }
        lsb->nxt.pop();
        nxt.pop();
        ticker_nxt = 0;
      } else ticker_nxt = ticker_cur + 1;
      return 0;
    }
    auto head = cur.front();
    if (head.cmd.type == LI) {
      return 1;
    }
    if (!Btype(head.cmd.type)) {
      if(Stype(head.cmd.type) || Ltype(head.cmd.type)) 
        ticker_nxt = 1;
      if (head.cmd.type == JALR)
        *reset = true, pc_nxt = head.pcDest;
      if(head.cmd.rd != -1)
        commit(head, reg); //
    } else {
      if (head.value != head.cmd.jp) {
        *reset = true;
        pc_nxt = head.value ? head.cmd.pc + head.cmd.imm : head.cmd.pc + 4;
      }
    }
    nxt.pop();
  }
  return 0;
}
void RS::alu(int value, int id) {
  for (int i = 0; i < arraycap; ++i)
    if (cur[i].busy) {
      if (cur[i].Qj == id) {
        nxt[i].Qj = -1, nxt[i].Vj = value;
      } // rs如何得到新值？
      if (cur[i].Qk == id) {
        nxt[i].Qk = -1, nxt[i].Vk = value;
      } // rs如何得到新值？
      if (cur[i].robID == id)
        nxt[i].busy = false;
    }
}
void RS::execute(ALU *alu) {
  if (alu_id_cur != -1) {
    nxt[alu_id_cur].busy = false;
  } else
    alu->nxt = insNode{};
  // R B JALR
  alu_id_nxt = cur.find();
}
bool IQ::fetch(const int &clk_, MEM* Mem) {
  if (!cur.full() && !cur_jalr) {
    unsigned int code = Mem->loadIns(pc_pred);
    ins cmd = decodeIns(code, pc_pred);
    cmd.clk = clk_;
    cmd.pc = pc_pred;
    cmd.output();
    if (cmd.type != NIL) {
      if (Btype(cmd.type)) {
        if (BP.predictor(pc_pred))
          pc_pred = pc_pred + cmd.imm - 4, cmd.jp = true;
      } else
        switch (cmd.type) {
        case JAL:
          pc_pred = pc_pred + cmd.imm - 4;
          break;
        case JALR:
          nxt_jalr = true;
          break;
        default:
          break;
        }
      nxt.push(cmd);
      pc_pred = pc_pred + 4;
      return true;
    } else
      assert(0);
  }
  return false;
}

}