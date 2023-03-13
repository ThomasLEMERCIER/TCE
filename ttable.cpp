#include "ttable.hpp"

TranspositionTable TT;

void TranspositionTable::clear() {
  memset(this, 0, sizeof(TranspositionTable));
}

int TranspositionTable::probe(Position* pos, TTEntry& tte) {
  tte = table[pos->hash_key % hash_size];

  if (tte.key == pos->hash_key)
    return 1;
  return 0;
}

void TranspositionTable::write_entry(Position* pos, int flag, int score, int depth, Move move) {
  // always replace scheme
  TTEntry *hash_entry = &table[pos->hash_key % hash_size];

  // store mate score independently from distance to root node
  if (score < - MATE_VALUE) score -= pos->ply;
  if (score > MATE_SCORE) score += pos->ply;

  hash_entry->key = pos->hash_key;
  hash_entry->depth = depth;
  hash_entry->flag = flag;
  hash_entry->score = score;
  hash_entry->best_move = move;
}