#include"memory.hpp"

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
  unsigned int MEM::read(int pos, int len) {
    unsigned int res = 0;
    for(int i = 0; i < len; ++i) {
      res |= (mem[pos++] << (i*8));
    }
    return res;
  }
  void MEM::write(int pos, int len, unsigned int dat){
    if(!pos) return;
    for(int i = 0; i < len; ++i){
      mem[pos++] = (dat & 0xFF);
      dat >>= 8;
    }
  }