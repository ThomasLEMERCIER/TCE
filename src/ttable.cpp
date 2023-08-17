#include "ttable.hpp"

#include <cstring>    // memset

TranspositionTable TT;

void TranspositionTable::clear() {
  memset(this, 0, sizeof(TranspositionTable));
}

bool TranspositionTable::probe(Position* pos, TTEntry& tte) {
  // return true if the position is found in the table

  // retrieve the entry from the table and set it to tte
  tte = table[pos->hash_key % hash_size];

  // check if the hash key matches
  if (tte.key == pos->hash_key)
    return true;
  return false;
}

void TranspositionTable::write_entry(Position* pos, int flag, Score score, int depth, Move move) {
  // always replace scheme
  TTEntry *hash_entry = &table[pos->hash_key % hash_size];

  // store mate score independently of distance to root node
  if (score <= -MATE_IN_MAX_PLY) score -= pos->ply;
  if (score >= MATE_IN_MAX_PLY) score += pos->ply;

  hash_entry->key = pos->hash_key;
  hash_entry->depth = depth;
  hash_entry->flag = flag;
  hash_entry->score = score;
  hash_entry->best_move = move;
}