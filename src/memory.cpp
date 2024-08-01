#include"memory.hpp"
#include <cstdio>
namespace CPU{

inline int b2hex(char c){
  if(isdigit(c)) return c - '0';
  else return c - 'A' + 10;
  }

unsigned int MEM::loadIns(unsigned int pc) {
  unsigned int res = 0;
  for (int i = 0; i < 4; ++i) {
    res |= (mem[pc++] << (i * 8));
    // std::cout << mem[pc] << " ";
  }
  // std::cout << res << std::endl;
  return res;
}
void MEM::loadInput(){
  unsigned int pos = 0;
  char c = getchar(); //std::cerr << c << std::endl;
  //std::cerr << "-----------Loadinput------------" << std::endl;
  while(c != EOF) {
    while(c != EOF && c != '@' && !isdigit(c) && !isalpha(c)) c = getchar();
    if(c == EOF) break;
    if(c == '@') {
      c = getchar();
      pos = 0;
      while(isdigit(c) || isalpha(c)) pos = (pos << 4) + b2hex(c), c = getchar();
      //std::cerr << pos << std::endl;
    } else {
      while(isdigit(c) || isalpha(c)) mem[pos] = (mem[pos] << 4) + b2hex(c), c = getchar();
      //std::cerr << "mem[" << pos << "] = " << int(mem[pos]) << std::endl; 
      ++pos;
    }
  }
}
  unsigned int MEM::read(int pos, int len) {
    unsigned int res = 0;
    for(int i = 0; i < len; ++i) {
      res |= (mem[pos++] << (i*8));
    }
    return res;
  }
  void MEM::write(int pos, int len, unsigned int dat){
    std::cout << "store" << pos << " " << len << " " << dat << std::endl;
    for(int i = 0; i < len; ++i){
      mem[pos++] = (dat & 0xFF);
      dat >>= 8;
    }
  }
}