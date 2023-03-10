#ifndef RNG_H_INCLUDED
#define RNG_H_INCLUDED

#include "definition.hpp"

class RNG {
public:
  RNG(unsigned int seed) : s(seed) { }

  unsigned int s;

  unsigned int rand32();
  U64 rand64();

  U64 magic_rand();
};

#endif