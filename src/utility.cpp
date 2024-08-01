#include "utility.hpp"
#include "config.hpp"

namespace CPU {
extern int pc_nxt, pc_pred;
pc_predictor BP;
void ALU::execute(ROB *rob, RS *rs, LSB *lsb, regfile *reg) {
  if (in.op == NIL) {
    rob->set_nxt = -1;
    return;
  }
  //std::cerr << "ALU "; in.output();
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
      // 特判 Vk 是 imm 
      rob->nxt[in.robID].pcDest = in.Vj + in.imm;
      //std::cerr << "JALR ready " << "v"<<in.Vj<<" imm"<<" "<<in.imm<<" ";  
      rob->nxt[in.robID].cmd.output();
    } break;
    default:
      assert(0);
    }
  }
  rob->alu(result, in.robID); // execute is_busy = false;
}

void ROB::alu(int value, int id) {
  nxt[id].value = value;
  nxt[id].ready = true;
  set_nxt = id;
}
// alu -> rs lsb
//     -> lsb
void ROB::commit(robNode rb, regfile* reg) {

  //std::cerr << "cmit [" << rb.cmd.robID << "]=" << rb.value << std::endl;
  reg->rob(rb.cmd.rd, rb.value, rb.cmd.robID);
  pc_nxt = rb.cmd.pc + 4;
  //std::cerr << rb.cmd.pc << std::endl;
}
bool ROB::execute(regfile *reg, IQ *iq, RS* rs, LSB* lsb, MEM* Mem, bool *reset) {
  if(set_cur != -1) {
    rs->alu(cur[set_cur].value, set_cur);  // rd = rs1 + rs2 何时标记 ？writeback
                              // 但未commit的时候 如果发现当前
    lsb->alu(cur[set_cur].value, set_cur);
  }
  if (!cur.empty() && cur.front().ready) {
    if(ticker_cur) {
      //std::cerr << "ticker "<<ticker_cur << std::endl;
      if(ticker_cur == 3) {
        // load instruction
        //std::cerr << "commit S"; 
        cur.front().cmd.output();
        // commit倒计时结束 已经写完，可以出队
        // S指令 直接读取当前的寄存器中数据
        // -> 执行完成发给队头，队头出队
        auto node = lsb->cur.front();
        // rob 接受 mem 完成的 store
        auto cmd = cur.front().cmd;
        if(!Stype(node.op)) assert(0);
        {
          // execute Store 操作 取当前的数
          if (node.op == SB)
            Mem->write(reg->cur[cmd.rs1].value + node.imm, 1, reg->cur[cmd.rs2].value);
          else if (node.op == SH)
            Mem->write(reg->cur[cmd.rs1].value + node.imm, 2, reg->cur[cmd.rs2].value);
          else if (node.op == SW)
            Mem->write(reg->cur[cmd.rs1].value + node.imm, 4, reg->cur[cmd.rs2].value);
        }
        // lsb 接受到 rob;
        {
          lsb->ticker_nxt = 2;
        }
        // 更改自身状态
        nxt.pop();
        ticker_nxt = 0;
      } else ticker_nxt = ticker_cur + 1;
      return 0;
    }
    // LI
    auto head = cur.front();
    // for(int i = 0; i < 32; ++i)
    //   std::cout << reg->cur[i].value << " ";
    // std::cout << std::endl;
    // head.cmd.output(std::cout);
    if (head.cmd.type == LI) {
      return 1;
    }
    if (!Btype(head.cmd.type)) {
      // 新进入 S 
      if(Stype(head.cmd.type)) 
        //std::cerr << "ticker S ", cur.front().cmd.output(),
        // lsb 设置
        lsb->ticker_nxt = 1,
        ticker_nxt = 1;
      // else  // R RI JALR等 和load没有区别
      //   std::cerr << "commit ", cur.front().cmd.output();
      if(head.cmd.rd != -1)
        commit(head, reg); //
      if (head.cmd.type == JALR)
        *reset = true, pc_nxt = head.pcDest, iq->nxt_jalr = false;// std::cerr << "pc_nxt = "<< pc_nxt << " " <<std::endl;
    } else {
      if (head.value != head.cmd.jp) {
        *reset = true;
        pc_nxt = head.value ? head.cmd.pc + head.cmd.imm : head.cmd.pc + 4;
      }
    }
    if(!ticker_nxt)
      nxt.pop();
  }
  // } else if(!cur.empty()) {
  //   std::cerr << "FAIL [" << (cur.head+1)%32 << "]"; cur.front().cmd.output();
  //   system("pause");
  // }
  return 0;
}
void RS::alu(int value, int id) {
//   if(value == 8 && id == 1)
// std::cerr<<233;
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

void LSB::output() {
  for (int i = (cur.head + 1) % 32; i != cur.tail; (i = (i+1)%32))
     {
      std::cerr << "LSB[" <<i << "] {" << cur[i].robID << " " << "{" << cur[i].Qj << "," << cur[i].Qk << "}" << "}\n"; 
    }
}
void RS::output() {
  for (int i = 0; i < arraycap; ++i)
    if (cur[i].busy) {
      std::cerr << "RS[" <<i << "] {" << cur[i].robID << " " << cur[i].pc << "{" << cur[i].Qj << "," << cur[i].Qk << "}" << "}\n"; 
    }
}
void RS::execute(ALU *alu) {
  if (alu_id_cur != -1) {
    nxt[alu_id_cur].busy = false;
    alu->nxt = cur[alu_id_cur];
  } else
    alu->nxt = insNode{};
  // R B JALR
  alu_id_nxt = cur.find(alu_id_cur);
}
bool IQ::fetch(const int &clk_, MEM* Mem) {
  // std::cerr << "CLOCK=" << clk_ << std::endl;
  if (!cur.full() && !cur_jalr) {
    unsigned int code = Mem->loadIns(pc_pred);
    ins cmd = decodeIns(code, pc_pred);
    cmd.clk = clk_;
    cmd.pc = pc_pred;
    //std::cerr << "FETCH" << " " << clk_ << " "; cmd.output();
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