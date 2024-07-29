#ifndef MEMORY_HPP
#define MEMORY_HPP
#include <cstdio>
#include <cctype>
#include <iostream>
const int MEMSIZE = 5E6;
struct MEM{
  // TODO clk
  unsigned char mem[MEMSIZE];
  bool busy_cur = false, busy_nxt = false;
  void reset(){busy_cur = busy_nxt = false;}
  void update(){busy_cur = busy_nxt;}
  inline int b2hex(char c){
  if(isdigit(c)) return c - '0';
  else return c - 'A' + 10;
}
inline unsigned int loadIns(unsigned int pc) {
  unsigned int res = 0;
  for (int i = 0; i < 4; ++i) {
    res |= (mem[pc++] << (i * 8));
    // std::cout << mem[pc] << " ";
  }
  // std::cout << res << std::endl;
  return res;
}
inline void loadInput(){
  unsigned int pos = 0;
  int c;
  puts("-----------Loadinput------------");
  while((c = getchar()) != EOF) {
    printf("%c ", c);
    while(c != EOF && c != '@' && !isdigit(c) && !isalpha(c)) c = getchar();
    if(c == EOF) break;
    if(c == '@') {
      c = getchar();
      while(isdigit(c) || isalpha(c)) pos = (pos << 4) + b2hex(c), c = getchar();
    } else {
      while(isdigit(c) || isalpha(c)) mem[pos] = (mem[pos] << 4) + b2hex(c), c = getchar();
      ++pos;
      std::cout << "mem[" << pos << "] = " << mem[pos] << std::endl; 
    }
  }
}
  inline unsigned int read(int pos, int len) {
    unsigned int res = 0;
    for(int i = 0; i < len; ++i) {
      res |= (mem[pos++] << (i*8));
    }
    return res;
  }
  inline void write(int pos, int len, unsigned int dat){
    if(!pos) return;
    for(int i = 0; i < len; ++i){
      mem[pos++] = (dat & 0xFF);
      dat >>= 8;
    }
  }
};

#endif