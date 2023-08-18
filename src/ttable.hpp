#pragma once

#include "definition.hpp"
#include "position.hpp"

constexpr int hash_size = 800000;

enum TTFlag : int { ExactFlag, UpperBound, LowerBound };

struct TTEntry {
  U64 key;        // hash key of position
  int depth;      // depth to get value
  TTFlag flag;    // flag for the type of node (fail-high/fail-low/PV)
  Score score;    // score (beta/alpha/PV)
  Move best_move; // best move found
};

class TranspositionTable {
public:
  TTEntry table[hash_size];

  bool probe(Position* pos, TTEntry& tte);
  void write_entry(Position* pos, TTFlag flag, Score score, int depth, Move move);
  void clear();
};

extern TranspositionTable TT;
