#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "config.hpp"
#include "memory.hpp"

// J 指令未知当前PC ? PC_pred是否相符？
// struct 类中互不知道接口？ A: 调用指针获取

// 三类线，1.cur = nxt(self update) 2. cur->other's nxt (sender) 3. nxt <-
// other's cur (receiver)
namespace CPU {

struct ALU;
struct ROB;
struct RS;
struct IQ;
struct LSB;
struct regfile;

struct ALU {
  insNode in, nxt;
  inline void update() { in = nxt; }
  inline void reset() { in = nxt = insNode{}; }
  // send result to ROB I,R: calc_result; B: 0,1; J、U ROB-> COMMIT
  // to ROB -> RS, REGF 解除依赖
  void execute(ROB *, RS *, LSB *, regfile *);
};
const int robcap = 32;
struct ROB {
  queue<robNode, robcap> cur, nxt;
  int ticker_cur, ticker_nxt;
  int snode_cur, snode_nxt;
  void update() { cur = nxt; ticker_cur = ticker_nxt; snode_cur = snode_nxt;}
  // receiver change nxt

  // receive from alu to undo relience
  void alu(int value, int id);
  void commit(robNode, regfile*);
  bool execute(regfile *, IQ *, LSB*, MEM*, bool *);
  inline void reset() { nxt.head = nxt.tail = 0; ticker_cur = ticker_nxt = 0, snode_cur = snode_nxt = -1;}
  // sender: change other's nxt
};

struct RS {
  array cur, nxt;
  int alu_id_cur, alu_id_nxt;

  RS() {
    alu_id_cur = alu_id_nxt = -1;
    // cur 表示下一周期 alu 是否是忙的 当cur = false时才可以给alu_nxt发指令;
    // 指令周期都是1，不用管？
  }
  inline void update() {
    cur = nxt;
    alu_id_cur = alu_id_nxt;
  }
  // 报错清空
  //
  inline void reset() {
    for (int i = 0; i < arraycap; ++i) {
      nxt[i] = {};
    }
    alu_id_cur = alu_id_nxt = -1;
  }
  void alu(int value, int id);
  void execute(ALU *alu);
};
struct IQ {
  queue<ins, 32> cur, nxt;
  // successfully push return true
  bool cur_jalr, nxt_jalr;
  IQ() { cur_jalr = nxt_jalr = false; }
  void update() {
    cur = nxt;
    cur_jalr = nxt_jalr;
  }
  bool fetch(const int &, MEM*);
  inline void reset() {
    nxt.head = nxt.tail = 0;
    cur_jalr = nxt_jalr = false;
  }
};
struct LSB {
  queue<insNode, 8> cur, nxt;
  int ticker_cur = 0, ticker_nxt = 0;
  void update() {
    cur = nxt;
    ticker_cur = ticker_nxt = 0;
  }
  void reset() {
    nxt.head = nxt.tail = 0;
    ticker_cur = ticker_nxt = 0;
  }
  void alu(int value, int id) {
    for (int i = 0; i < arraycap; ++i)
      if (nxt[i].busy) {
        if (nxt[i].Qj == id) {
          nxt[i].Qj = -1, nxt[i].Vj = value;
        } // rs如何得到新值？
        if (nxt[i].Qk == id) {
          nxt[i].Qk = -1, nxt[i].Vk = value;
        } // rs如何得到新值？
        if (nxt[i].robID == id)
          nxt[i].busy = false;
      }
  }
  void execute(regfile *reg, ROB *rob, MEM* Mem) {
    if (ticker_cur || rob->snode_cur != -1) {
      if (ticker_cur == 3) {
        ticker_nxt = 0;
        Mem->busy_nxt = false;
        auto node = cur.front();
        node.busy = false;
        cur.pop();
        rob->nxt[node.robID].ready = true;
      } else
        ticker_nxt = ticker_cur + 1;
    } else if (!cur.empty()) {
      auto node = cur.front();
      if (node.Qj == -1 && node.Qk == -1) {
        rob->nxt[node.robID].ready = true;
      }
    }
  }
};
} // namespace CPU
#endif