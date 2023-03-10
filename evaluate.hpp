#ifndef EVALUATE_H_INCLUDED
#define EVALUATE_H_INCLUDED

#include "definition.hpp"
#include "bitboard.hpp"
#include "position.hpp"

constexpr int material_score[12] = {
  100,      // white pawn score
  300,      // white knight score
  350,      // white bishop score
  500,      // white rook score
  1000,     // white queen score
  10000,    // white king score
  -100,     // black pawn score
  -300,     // black knight score
  -350,     // black bishop score
  -500,     // black rook score
  -1000,    // black queen score
  -10000,   // black king score
};
constexpr int pawn_score[64] = {
  90,  90,  90,  90,  90,  90,  90,  90,
  30,  30,  30,  40,  40,  30,  30,  30,
  20,  20,  20,  30,  30,  30,  20,  20,
  10,  10,  10,  20,  20,  10,  10,  10,
    5,  5,  10,  20,  20,   5,   5,   5,
    0,   0,   0,   5,   5,   0,   0,   0,
    0,   0,   0, -10, -10,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};
constexpr int knight_score[64] = {
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,  10,  10,   0,   0,  -5,
  -5,   5,  20,  20,  20,  20,   5,  -5,
  -5,  10,  20,  30,  30,  20,  10,  -5,
  -5,  10,  20,  30,  30,  20,  10,  -5,
  -5,   5,  20,  10,  10,  20,   5,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5, -10,   0,   0,   0,   0, -10,  -5
};
constexpr int bishop_score[64] = {
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,  20,   0,  10,  10,   0,  20,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,  10,   0,   0,   0,   0,  10,   0,
  0,  30,   0,   0,   0,   0,  30,   0,
  0,   0, -10,   0,   0, -10,   0,   0
};
constexpr int rook_score[64] = {
  50,  50,  50,  50,  50,  50,  50,  50,
  50,  50,  50,  50,  50,  50,  50,  50,
  0,    0,  10,  20,  20,  10,   0,   0,
  0,    0,  10,  20,  20,  10,   0,   0,
  0,    0,  10,  20,  20,  10,   0,   0,
  0,    0,  10,  20,  20,  10,   0,   0,
  0,    0,  10,  20,  20,  10,   0,   0,
  0,    0,   0,  20,  20,   0,   0,   0

};
constexpr int king_score[64] = {
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   5,   5,   5,   5,   0,   0,
  0,   5,   5,  10,  10,   5,   5,   0,
  0,   5,  10,  20,  20,  10,   5,   0,
  0,   5,  10,  20,  20,  10,   5,   0,
  0,   0,   5,  10,  10,   5,   0,   0,
  0,   5,   5, -10, -10,   0,   5,   0,
  0,   0,  5,   0, -15,   0,  10,   0
};

constexpr Square mirror_score[64] = {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

extern Bitboard isolated_masks[SQUARE_NB];
extern Bitboard white_passed_masks[SQUARE_NB];
extern Bitboard black_passed_masks[SQUARE_NB];

constexpr int double_pawn_penalty = -10;
constexpr int isolated_pawn_penalty = -10;
constexpr int passed_pawn_bonus[8] = { 200, 150, 100, 0, 0, 0, 0, 0 }; 

constexpr int semi_open_file_score = 10;
constexpr int open_file_score = 15;
constexpr int king_shield_bonus = 5;

void init_evaluation_masks();
int evaluate(Position* pos);

#endif
