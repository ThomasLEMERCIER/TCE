#include "move.hpp"

#include <iostream>

Move encode_move(Square source, Square target, Piece piece, Piece promoted, int capture_f, int double_f, int enpassant_f, int castling_f) {
  return source | (target << 6) | (piece << 12) | (promoted << 16) | (capture_f << 20) | (double_f << 21) | (enpassant_f << 22) | (castling_f << 23);
}

Square get_move_source(Move move)   { return Square(move & 0x3f); }
Square get_move_target(Move move)   { return Square((move & 0xfc0) >> 6); }
Piece get_move_piece(Move move)     { return Piece((move & 0xf000) >> 12); }
Piece get_move_promoted(Move move)  { return Piece((move & 0xf0000) >> 16); }
bool get_move_capture_f(Move move)   { return (move & 0x100000); }
bool get_move_double_f(Move move)    { return (move & 0x200000); }
bool get_move_enpassant_f(Move move) { return (move & 0x400000); }
bool get_move_castling_f(Move move)  { return (move& 0x800000); }

void print_move(Move move) {
  if (move == UNDEFINED_MOVE)
    std::cout << "UNDEFINED_MOVE";
  else if (get_move_promoted(move))
    std::cout << square_to_coordinates[get_move_source(move)] << square_to_coordinates[get_move_target(move)] << promoted_pieces.at(get_move_promoted(move));
  else
    std::cout << square_to_coordinates[get_move_source(move)] << square_to_coordinates[get_move_target(move)];
}