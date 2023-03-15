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
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

constexpr int double_pawn_penalty = -10;
constexpr int isolated_pawn_penalty = -10;
constexpr int passed_pawn_bonus[8] = { 200, 150, 100, 0, 0, 0, 0, 0 }; 

constexpr int semi_open_file_score = 10;
constexpr int open_file_score = 15;
constexpr int king_shield_bonus = 5;

void init_evaluation_masks();
int evaluate(Position* pos);

#endif
