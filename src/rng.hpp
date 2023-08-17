#pragma once

#include "definition.hpp"

class RNG {
public:
  explicit RNG(unsigned int seed) : s(seed) { }

  unsigned int s;

  unsigned int rand32();
  U64 rand64();

  U64 magic_rand();
};
