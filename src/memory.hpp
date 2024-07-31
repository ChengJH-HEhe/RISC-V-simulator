#ifndef MEMORY_HPP
#define MEMORY_HPP
#include <cctype>
#include <cstdio>
#include <iostream>
const int MEMSIZE = 5E6;
struct MEM {
  // TODO clk
  unsigned char mem[MEMSIZE];
  bool busy_cur = false, busy_nxt = false;
  void reset() { busy_cur = busy_nxt = false; }
  void update() { busy_cur = busy_nxt; }
  unsigned int read(int pos, int len);
  void write(int pos, int len, unsigned int dat);
  unsigned int loadIns(unsigned int pc);
  void loadInput();
};

#endif