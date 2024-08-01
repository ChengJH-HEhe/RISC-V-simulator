#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <cassert>
#include <iostream>
// pc block

namespace CPU{
struct pc_predictor{
  bool predictor(unsigned int pc){
    return true;
  }
};

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


bool RBtype(Instype _);
bool Rtype(Instype _) ;
bool RItype(Instype _);
bool Btype(Instype _) ;
bool Ltype(Instype _);
bool Stype(Instype _) ;
const int Rop = 0b0110011, Iop = 0b0010011, Lop = 0b0000011, Sop = 0b0100011,
          Bop = 0b1100011;

struct ins {
  Instype type;
  unsigned int rs2, rs1, rd;
  bool flg1, flg2, jp; // 1 时不读， Q 记作 -1
  int imm, robID, pc;
  int clk;
  void output(std::ostream& ci = std::cerr);
  // Jump 记录 pc
};
const int regcap = 32;

struct regfile {
  struct regs {
    int Qi = -1;
    int value;
  } cur[regcap], nxt[regcap];
  regfile(){
    cur[0].value = nxt[0].value = 0;
  }
  inline void reset(){
    for(int i = 0; i < regcap; ++i)
      cur[i].Qi = nxt[i].Qi = -1;
  }
  inline void update() {
    for(int i = 0; i < regcap; ++i)
      cur[i] = nxt[i];
  }
  inline int getQ(int id) {
    return cur[id].Qi;
  }
  inline int getV(int id){
    return cur[id].value;
  }
  inline void chQ(int id, int rl){
    if(id == 0) return;
    nxt[id].Qi = rl;
  }
  inline void chV(int id, int v) {
    if(id == 0) return;
    nxt[id].value = v;
  }
  inline void rob(int id, int val,int rl) {
    chV(id, val);
    if(getQ(id) == rl) {
      chQ(id, -1);
    } else {
      //std::cerr << "rob fail " << getQ(id) << "!=" << rl << std::endl;
    }
  }
  inline void alu(int id, int robid) {
    if(id < 0) {

    std::cerr << id << std::endl;
    assert(id >= 0);
    }
    nxt[id].Qi = robid;
  }
  inline void output() {
    for(int i = 0; i < regcap; ++i)
      if(cur[i].value)
        std::cerr << "R[" << i << "]={" << cur[i].Qi << ", " << cur[i].value << "}" << std::endl;
  }
};
struct insNode {
  bool busy = false;
  bool ready = false;
  unsigned int Vj, Vk;
  int Qj = -1, Qk = -1;
  int clk, pc, rd;
  Instype op;
  int imm;
  unsigned int robID;
  insNode() {
    op = NIL;
    busy = false;
  }
  explicit insNode(const ins &_ins, const regfile& reg) {
    op = _ins.type;
    pc= _ins.pc;
    rd = _ins.rd;
    imm = _ins.imm;
    clk = _ins.clk;
    Qj = _ins.flg1 == 1 ? -1 : (reg.cur[_ins.rs1].Qi == -1 ? (Vj = reg.cur[_ins.rs1].value, -1): reg.cur[_ins.rs1].Qi);
    Qk = _ins.flg2 == 1 ? -1 : (reg.cur[_ins.rs2].Qi == -1 ? (Vk = reg.cur[_ins.rs2].value, -1): reg.cur[_ins.rs2].Qi);
    robID = _ins.robID;
    busy = true;
  }
  void output();
};
struct robNode {
  bool ready = false;
  unsigned int value, pcDest; // pcDest or calc result
  ins cmd;
  robNode() = default;
  robNode(const ins &cmd_, bool ready_ = false) {
    ready = ready_;
    cmd = cmd_;
  }
};

template <class datatype, int cap> class queue {
  datatype dat[cap];

public:
  int head = 0, tail = 1;

public:
  queue(){
    head = 0; tail = 1;
  }
  void clear() {
    for(int i = 0; i < cap; ++i)
      dat[i] = datatype{};
    head = 0, tail = 1;
  }
  bool full() { return head == tail; }
  bool empty() { return (head + 1 - tail) % cap == 0; }
  int frontNum() { return (head + 1) % cap; }
  int tailNum() { return tail; }
  datatype &operator[](const int &pos) { return dat[pos]; }
  datatype &front() { return dat[(head + 1) % cap]; }
  void push(const datatype &d) {
    dat[tail] = d;
    tail = (tail + 1) % cap;
  }
  void pop() { head = (head + 1) % cap; }
  void operator=(const queue &rhs) {
    head = rhs.head, tail = rhs.tail;
    for (int i = 0; i < cap; ++i)
      dat[i] = rhs.dat[i];
  }
};
const int arraycap = 8;
struct array {
  insNode dat[arraycap];
public:
  bool insert(const insNode &rhs) {
    for (int i = 0; i < arraycap; ++i)
      if (!dat[i].busy) {
        dat[i] = rhs;
        return true;
      }
    return false;
  }
  insNode &operator[](const int &pos) { return dat[pos]; }
  int find(int cur) {
    int res = -1, resage = -1;
    for (int i = 0; i < arraycap; ++i) {
      if(i == cur) continue;
      if (dat[i].busy && (dat[i].Qj == -1 && dat[i].Qk == -1) && (res == -1 || dat[i].clk < resage) )
        res = i, resage = dat[i].clk;
    }
    return res;
  }
  void operator=(const array& rhs) {
    for(int i = 0; i < arraycap; ++i) {
      dat[i] = rhs.dat[i];
    }
  }
  void reset(){
    for(int i = 0; i < arraycap; ++i) {
      dat[i] = insNode{};
    }
  }
};

inline int sign_extend(unsigned int nw, int highest_bit) {
  return (nw >> (highest_bit - 1) & 1)
             ? (nw | ((0xFFFFFFFF) >> highest_bit) << highest_bit)
             : nw;
}


// load immediate & determine the type
inline int R_calc(insNode res) {
  switch (res.op) {
  case ADD:
  case ADDI:
    return res.Vj + res.Vk;
  case SUB:
    return res.Vj - res.Vk;
  case AND:
  case ANDI:
    return res.Vj & res.Vk;
  case OR:
  case ORI:
    return res.Vj | res.Vk;
  case XOR:
  case XORI:
    return res.Vj ^ res.Vk;
  case SLL:
  case SLLI:
    return res.Vj << res.Vk;
  case SRL:
  case SRLI:
    return res.Vj >> res.Vk;
  case SRA:
  case SRAI:
    return sign_extend(res.Vj >> res.Vk, 32 - res.Vk);
  case SLTU:
  case SLTIU:
    return (unsigned int)(res.Vj) < (unsigned int)(res.Vk) ? 1 : 0;
  case SLT:
  case SLTI:
    return res.Vj < res.Vk ? 1 : 0;
  default:
    assert(0);
  }
}

inline int B_calc(insNode res) {
  switch (res.op) {
  case BEQ:
    return res.Vj == res.Vk;
    break;
  case BGE:
    return res.Vj >= res.Vk;
    break;
  case BGEU:
    return (unsigned int)(res.Vj) >= (unsigned int)(res.Vk);
    break;
  case BLT:
    return res.Vj < res.Vk;
    break;
  case BLTU:
    return (unsigned int)(res.Vj) < (unsigned int)(res.Vk);
    break;
  case BNE:
    return res.Vj != res.Vk;
    break;
  default:
    assert(0);
  }
}

inline void R(ins &res, int funct3, int funct7, unsigned int code) {
  res.flg1 = res.flg2 = 0; // rs1 rs2
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

inline void I(ins &res, int funct3, int funct7, unsigned int code) {
  res.flg2 = 1, res.flg1 = 0; // rd imm rs1
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
  if (SLLI <= res.type && res.type <= SRAI) { // I*
    res.imm = code >> 20 & 0x1F;
    res.imm = sign_extend(res.imm, 5);
  } else {
    res.imm = code >> 20;
    res.imm = sign_extend(res.imm, 12);
  }
}

inline void L(ins &res, int funct3, int funct7, unsigned int code) {
  res.flg1 = 0, res.flg2 = 1; // rs1
  switch (funct3) {
  case 0b000:
    res.type = LB;
    break;
  case 0b100:
    res.type = LBU;
    break;
  case 0b001:
    res.type = LH;
    break;
  case 0b101:
    res.type = LHU;
    break;
  case 0b010:
    res.type = LW;
    break;
  }
  res.imm = code >> 20;
  res.imm = sign_extend(res.imm, 12);
}

inline void S(ins &res, int funct3, int funct7, unsigned int code) {
  res.flg1 = 0, res.flg2 = 0; // rs1, rs2
  switch (funct3) {
  case 0b000:
    res.type = SB;
    break;
  case 0b001:
    res.type = SH;
    break;
  case 0b010:
    res.type = SW;
    break;
  }
  res.rd = -1;
  res.imm = ((code >> 25) << 5) | ((code >> 7) & 0x1F);
  res.imm = sign_extend(res.imm, 12);
}

inline void B(ins &res, int funct3, int funct7, unsigned int code) {
  res.flg1 = 0, res.flg2 = 0; // rs1 rs2
  res.rd = -1;
  switch (funct3) {
  case 0b000:
    res.type = BEQ;
    break;
  case 0b101:
    res.type = BGE;
    break;
  case 0b111:
    res.type = BGEU;
    break;
  case 0b100:
    res.type = BLT;
    break;
  case 0b110:
    res.type = BLTU;
    break;
  case 0b001:
    res.type = BNE;
    break;
  }
  res.imm = ((code >> 31 & 1) << 12) | (((code >> 25) & 0x3F) << 5) |
            ((code >> 7 & 1) << 11) | ((code >> 8 & 0xF) << 1);
  res.imm = sign_extend(res.imm, 13);
}

inline ins decodeIns(unsigned int code, unsigned int pc) {
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
  res.type = NIL;
  switch (opcode) {
  case Rop:
    R(res, funct3, funct7, code);
    break;
  case Iop:
    I(res, funct3, funct7, code);
    break;
  case Lop:
    L(res, funct3, funct7, code);
    break;
  case Bop:
    B(res, funct3, funct7, code);
    break;
  case Sop:
    S(res, funct3, funct7, code);
    break;
  default:
    if (opcode == 0b1101111) { // JAL
      res.type = JAL;          // J
      res.imm = ((code >> 31 & 1) << 20) | ((code >> 21 & 0x3FF) << 1) |
                ((code >> 20 & 1) << 11) | ((code >> 12 & 0xFF) << 12);
      res.imm = sign_extend(res.imm, 21);
      res.flg1 = res.flg2 = 1;
    } else if (opcode == 0b1100111) {
      res.type = JALR;
      res.flg1 = 0, res.flg2 = 1;
      res.imm = code >> 20;
      res.imm = sign_extend(res.imm, 12);
    } else {
      res.imm = (code >> 12) << 12;
      res.flg1 = res.flg2 = 1;
      if (opcode == 0b0010111) {
        res.type = AUIPC;
      } else if (opcode == 0b0110111) {
        res.type = LUI;
      } else {
        std::cerr << opcode << std::endl;
        assert(0);
      }
    }
  }
  return res;
}
}
#endif