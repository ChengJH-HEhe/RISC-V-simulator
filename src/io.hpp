#ifndef IO_HPP
#define IO_HPP

#include "config.hpp"
#include "memory.hpp"
#include <cstdio>
#include <cctype>
#include <iostream>

int b2hex(char c){
  if(isdigit(c)) return c - '0';
  else return c - 'A' + 10;
}
void loadInput(){
  unsigned int pos = 0;
  char c = getchar();
  while(c!= EOF) {
    while(c != EOF && c != '@' && !isdigit(c) && !isalpha(c)) c = getchar();
    if(c == EOF) break;
    if(c == '@') {
      while(isdigit(c) || isalpha(c)) pos = (pos << 4) + b2hex(c), c = getchar();
    } else {
      while(isdigit(c) || isalpha(c)) mem[pos] = (mem[pos] << 4) + b2hex(c), c = getchar();
      ++pos;
    }
  }
}
#endif