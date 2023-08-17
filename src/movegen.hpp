#pragma once

#include "definition.hpp"
#include "position.hpp"
#include "bitboard.hpp"
#include "move.hpp"

struct MoveList {

  MoveExt moves[MAX_MOVES];
  int move_count = 0;
};

template<Piece piece>
void generate_piece_moves(Position* pos, MoveList* move_list);
void generate_moves(Position* pos, MoveList* move_list);
template<Color c>
void generate_pawn_moves(Position* pos, MoveList* move_list);
void generate_castle_moves(Position* pos, MoveList* move_list);
void move_push(MoveList* move_list, Move move);

void print_move_list(MoveList* move_list);
