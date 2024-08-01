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
  int set_cur, set_nxt;
  queue<robNode, robcap> cur, nxt;
  int ticker_cur, ticker_nxt;
  void update() {
    cur = nxt;
    set_cur = set_nxt;
    ticker_cur = ticker_nxt;
  }
  // receiver change nxt
  // receive from alu to undo relience
  void alu(int value, int id);
  void commit(robNode, regfile*);
  inline void output() {
    for(int i = (cur.head+1)%robcap; i != cur.tail; (i = (i+1)%robcap)) 
      std::cerr << "rob[" << i << "] " << (cur[i].ready?"ready":""), cur[i].cmd.output();
  }
  bool execute(regfile *, IQ *,RS *, LSB *, MEM *, bool *);
  inline void reset() {
    set_cur = set_nxt = -1;
    cur.clear(), nxt.clear();
    ticker_cur = ticker_nxt = 0;
  }
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
  void output();
  inline void reset() {
    cur.reset(), nxt.reset();
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
  bool fetch(const int &, MEM *);
  inline void reset() {
    cur.clear(), nxt.clear();
    cur_jalr = nxt_jalr = false;
  }
};
struct LSB {
  queue<insNode, 32> cur, nxt;
  void output();
  int ticker_cur = 0, ticker_nxt = 0;
  int load_cur = 0, load_nxt = 0;
  void update() {
    cur = nxt;
    ticker_cur = ticker_nxt;
    load_cur = load_nxt;
  }
  void reset() {
    cur.clear(), nxt.clear();
    ticker_cur = ticker_nxt = 0;
    load_cur = load_nxt = 0;
  }
  void alu(int value, int id) {
    for (int i = (cur.head+1)%32; i != (cur.tail); i = (i+1)%32)
      {
        if (cur[i].Qj == id) {
          nxt[i].Qj = -1, nxt[i].Vj = value;
        }
        if (cur[i].Qk == id) {
          nxt[i].Qk = -1, nxt[i].Vk = value;
        }
        if (cur[i].robID == id)
          nxt[i].busy = false;
      }
  }
  void execute(regfile *reg, ROB *rob, RS* rs, MEM *Mem) {
    if (ticker_cur) {
      // lsb 接收到 rob 发现store已经结束，那么释放rs1, rs2依赖，
      // rs1, rs2在store的3个周期里由于顺序提交，regfile中的值一定要与Vj, Vk
      // 相同。因此在commit的时候直接读取reg_cur即可，提前存储的值未必保真
      // 而load 指令又和 RS没有区别？？？ 但为了顺序执行check队头还是保留吧
      if (ticker_cur == 2)
        nxt.front().busy = false, nxt.pop(), ticker_nxt = 0;
      // 1 执行中 2 执行完全
    } else if (load_cur) {
      if (load_cur == 3 ) {
        // 开始执行
        if(rob->set_nxt != -1) return;
        unsigned int value;
        auto node = cur.front();
        if (node.op == LB)
          value = Mem->read(node.Vj + node.imm, 1), value = sign_extend(value, 8);
        else if (node.op == LH)
          value = Mem->read(node.Vj + node.imm, 2), value = sign_extend(value, 16);
        else if (node.op == LW)
          value = Mem->read(node.Vj + node.imm, 4), value = sign_extend(value, 32);
        else if(node.op == LBU)
          value = Mem->read(node.Vj + node.imm, 1);
        else if(node.op == LHU)
          value = Mem->read(node.Vj + node.imm, 2);
        else assert(0);
        // like alu did
        // std::cerr << "load lsb[" << cur.head << " " << cur.tail << "]->";
        // std::cerr << "load rob[" << rob->cur.head << " " << rob->cur.tail << "]"; 
        // std::cerr << "load change[" << node.robID << "] to true\n";
        rob->alu(value, cur.front().robID);
        // rob->alu(value, cur.front().robID); // execute is_busy = false;
        // rs->alu(value, cur.front().robID);  // rd = rs1 + rs2 何时标记 ？writeback
                              // 但未commit的时候 如果发现当前
        //alu(value, cur.front().robID);
        nxt.front().busy = false, nxt.pop();
        load_nxt = 0;
      } else
        load_nxt = load_cur + 1;
    } else if (!cur.empty()) {
      auto node = cur.front();
      if (Ltype(node.op)) {
          if (node.Qj == -1 && node.Qk == -1) {
            // Load指令 解除rd依赖，此前rd依赖可以被更改？
            // 那就应该改的和store不一样 rd rob处理 约定load
            // LSB内部自行卡住，rob不会显示ready，等待mem返回给lsb时再将rob改成ready，然后自行删去LSB队头。
            // 此时ticker_cur始终是0 ROB只会解除依赖
            load_nxt = 1;
          }
      } else {
        if (!Stype(node.op))
          assert(0);
        // rob 接收到 开始S的标志是该指令ready lsb开始s的标志时ticker_cur = 1
        // 如果=2说明可以自行删去了 rob无权更改lsb
        // 但真正完成在rob.ticker_cur=3的时候
        ticker_nxt = 1;
        
        rob->nxt[node.robID].ready = true;
        // std::cerr << "lsb[" << cur.head << " " << cur.tail << "]->";
        // std::cerr << "rob[" << rob->cur.head << " " << rob->cur.tail << "]"; 
        // std::cerr << "change[" << node.robID << "] to true\n";
      }
    }
  }
};
} // namespace CPU
#endif