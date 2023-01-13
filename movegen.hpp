#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "definition.hpp"
#include "position.hpp"
#include "bitboard.hpp"
#include "move.hpp"

struct MoveList {

  Move moves[MAX_MOVES];
  int move_count = 0;
};

void generate_moves(MoveList* move_list, Position* pos);

void add_move(MoveList* move_list, Move move);
void print_move_list(MoveList* move_list);

#endif