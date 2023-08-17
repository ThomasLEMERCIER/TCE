#pragma once

#include "definition.hpp"
#include "position.hpp"
#include "move.hpp"

constexpr int hash_size = 800000;

enum TTFlag : int { ExactFlag, UpperBound, LowerBound };

struct TTEntry {
  U64 key;          // hash key of position
  int depth;        // depth to get value
  int flag;         // flag for the type of node (fail-high/fail-low/PV)
  int score;        // score (beta/alpha/PV)
  Move best_move;   // best move found
};

class TranspositionTable {
public:
  TTEntry table[hash_size];

  int probe(Position* pos, TTEntry& tte);
  void write_entry(Position* pos, int flag, int score, int depth, Move move);
  void clear();
};

extern TranspositionTable TT;
