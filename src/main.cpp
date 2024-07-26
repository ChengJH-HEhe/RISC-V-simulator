#include "io.hpp"
#include "config.hpp"
#include "sequential.hpp"

int main() {
  freopen("array_test1.data", "r", stdin);
  loadInput();
  int clk = 0;
  for(;;){
    ++clk;
    updateAll();
    fetch();
    if(toyCPU.halt) break;
    execute();
  }
}