#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include <string.h>

#include "definition.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "rng.hpp"

extern U64 piece_keys[12][64];
extern U64 side_key;
extern U64 enpassant_keys[64];
extern U64 castle_keys[64];

class Position {
public:
  Position() = default;
  Position(Position* pos);


  Bitboard bitboards[PIECE_NB];
  Bitboard occupancies[OCCUPANCY_NB];

  Color side;
  Square enpassant;
  int castle_rights;

  U64 hash_key;
  U64 repetition_table[MAX_PLY_GAME];
  int repetition_index;

  int ply;

  void set(const char* fenStr);
};

int is_square_attacked(Position* pos, Square square, Color side);
int make_move(Position* pos, Move move, Move_Type move_flag);
void make_null_move(Position* pos);
U64 generate_hash_key(Position* pos);
void init_random_keys();
void print_board(Position* pos);

void copy_position(Position* dst, Position* src);

#endif