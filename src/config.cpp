#include "config.hpp"

bool CPU::RBtype(Instype _){
  return (BEQ <= _ && _ <= BNE) || (XOR <= _ && _ <= SLTIU); 
}
bool CPU::Rtype(Instype _) {
  return (XOR <= _ && _ <= SLTIU);
}
bool CPU::RItype(Instype _) {
  return (ADDI <= _ && _ <= SLTIU);
}
bool CPU::Btype(Instype _) {
  return (BEQ <= _ && _ <= BNE);
}
bool CPU::Ltype(Instype _) {
  return (LB <= _ && _ <= LW);
}
bool CPU::Stype(Instype _) {
  return (SB <= _ && _ <= SW);
}