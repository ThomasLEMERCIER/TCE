#include "rng.hpp"

unsigned int RNG::rand32() {
  // xor shift algorithm
  s ^= s << 13; s ^= s >> 17; s ^= s << 5;
  return s;
}

U64 RNG::rand64() {
  U64 n1, n2, n3, n4;
  // init random numbers keeping only the first 16 LSB
  n1 = (U64)(rand32()) & 0xFFFF;
  n2 = (U64)(rand32()) & 0xFFFF;
  n3 = (U64)(rand32()) & 0xFFFF;
  n4 = (U64)(rand32()) & 0xFFFF;
  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 RNG::magic_rand() {
  return rand64() & rand64() & rand64();
}