#pragma once

#include "definition.hpp"

/* attack bits binary representation

  binary                                                                        hexidecimal

  0000 0000 0000 0000 0011 1111   source square (6 bits) (max val 63 (2^6-1))   0x3f
  0000 0000 0000 1111 1100 0000   target square (6 bits) (max val 63 (2^6-1))   0xfc0
  0000 0000 1111 0000 0000 0000   piece (4 bits) (max val 11 (2^4-1))           0xf000
  0000 1111 0000 0000 0000 0000   promoted piece (4 bits)                       0xf0000
  0001 0000 0000 0000 0000 0000   capture flag (1 bit)                          0x100000
  0010 0000 0000 0000 0000 0000   double push flag (1 bit)                      0x100000
  0100 0000 0000 0000 0000 0000   enpassant flag (1 bit)                        0x400000
  1000 0000 0000 0000 0000 0000   castling flag (1 bit)                         0x800000

  ==> need minimum 24 bits to store a move, so we use 32 bits (4 bytes)
*/

struct MoveExt
{
  Move move;
  int score;
};


Move encode_move(Square source, Square target, Piece piece, Piece promoted, int capture_f, int double_f, int enpassant_f, int castling_f);

Square get_move_source(Move move);
Square get_move_target(Move move);
Piece get_move_piece(Move move);
Piece get_move_promoted(Move move);
bool get_move_capture_f(Move move);
bool get_move_double_f(Move move);
bool get_move_enpassant_f(Move move);
bool get_move_castling_f(Move move);

void print_move(Move move);
