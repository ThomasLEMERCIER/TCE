#include "movegen.hpp"

#include "bitboard.hpp"

#include <iostream>

void generate_moves(Position* pos, MoveList* move_list) {
  move_list->move_count = 0;

  generate_castle_moves(pos, move_list);
  if (pos->side == Color::WHITE) {
    generate_pawn_moves<Color::WHITE>(pos, move_list);
    generate_piece_moves<Piece::WN>(pos, move_list);
    generate_piece_moves<Piece::WB>(pos, move_list);
    generate_piece_moves<Piece::WR>(pos, move_list);
    generate_piece_moves<Piece::WQ>(pos, move_list);
    generate_piece_moves<Piece::WK>(pos, move_list);
  } else {
    generate_pawn_moves<Color::BLACK>(pos, move_list);
    generate_piece_moves<Piece::BN>(pos, move_list);
    generate_piece_moves<Piece::BB>(pos, move_list);
    generate_piece_moves<Piece::BR>(pos, move_list);
    generate_piece_moves<Piece::BQ>(pos, move_list);
    generate_piece_moves<Piece::BK>(pos, move_list);
  }
}

template<Piece piece>
void generate_piece_moves(Position* pos, MoveList* move_list) {
  Bitboard bb = pos->bitboards[piece];
  Bitboard attacks;
  Square source_square, target_square;

  while (bb) {
    source_square = get_lsb_index(bb);
    attacks = get_attacks_bb<piece>(source_square, pos->occupancies[Color::BOTH]) & ((pos->side == Color::WHITE) ? ~pos->occupancies[Color::WHITE] : ~pos->occupancies[Color::BLACK]);

    while (attacks)
    {
      target_square = get_lsb_index(attacks);

      if (!get_bit((pos->side == Color::WHITE) ? pos->occupancies[Color::BLACK] : pos->occupancies[Color::WHITE], target_square))
        move_push(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
      else
        move_push(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

      pop_bit(attacks, target_square);
    }
    pop_bit(bb, source_square);
  }
}

template <Color c>
void generate_pawn_moves(Position* pos, MoveList* move_list) {
  constexpr Piece piece = (c == Color::WHITE) ? Piece::WP : Piece::BP;
  constexpr Direction forward  = (c == Color::WHITE) ? Direction::UP : Direction::DOWN;
  constexpr Direction backward  = (c == Color::WHITE) ? Direction::DOWN : Direction::UP;
  constexpr Bitboard RelativeRank7 = (c == Color::WHITE) ? RANK_7_BB : RANK_2_BB;
  constexpr Bitboard RelativeRank3 = (c == Color::WHITE) ? RANK_3_BB : RANK_6_BB;

  Bitboard pawns = pos->bitboards[piece];
  Bitboard pawnsOn7 = pawns & RelativeRank7;
  Bitboard pawnsNotOn7 = pawns & ~RelativeRank7;

  Bitboard emptySquares = ~pos->occupancies[Color::BOTH];
  Bitboard enemies = (pos->side == Color::WHITE) ? pos->occupancies[Color::BLACK] : pos->occupancies[Color::WHITE];

  Square source_square, target_square;

  Bitboard b1, b2, b3;

  // Quiet Move
  b1 = shift<forward>(pawnsNotOn7) & emptySquares; // One forward
  b2 = shift<forward>(b1 & RelativeRank3) & emptySquares; // Double push on second rank = (one forward rank 2 + one forward rank 3)
  while (b1) {
    target_square = get_lsb_index(b1);
    source_square = shift<backward>(target_square);
    move_push(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
    pop_bit(b1, target_square);
  }

  while (b2) {
    target_square = get_lsb_index(b2);
    source_square = shift<backward>(shift<backward>(target_square));
    move_push(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 1, 0, 0));
    pop_bit(b2, target_square);
  }

  // Promotions and underpromotions
  if (pawnsOn7) {
    b1 = shift<RIGHT>(shift<forward>(pawnsOn7)) & enemies; // capture promotion
    b2 = shift<LEFT>(shift<forward>(pawnsOn7)) & enemies; // capture promotion
    b3 = shift<forward>(pawnsOn7) & emptySquares; // forward promotion

    while (b1) {
      target_square = get_lsb_index(b1);
      source_square = shift<LEFT>(shift<backward>(target_square));
      for (auto piece_prom : (pos->side == Color::WHITE) ? WhitePromPiece : BlackPromPiece)
        move_push(move_list, encode_move(source_square, target_square, piece, piece_prom, 1, 0, 0, 0));            
      pop_bit(b1, target_square);
    }

    while (b2) {
      target_square = get_lsb_index(b2);
      source_square = shift<RIGHT>(shift<backward>(target_square));
      for (auto piece_prom : (pos->side == Color::WHITE) ? WhitePromPiece : BlackPromPiece)
        move_push(move_list, encode_move(source_square, target_square, piece, piece_prom, 1, 0, 0, 0));            
      pop_bit(b2, target_square);
    }

    while (b3) {
      target_square = get_lsb_index(b3);
      source_square = shift<backward>(target_square);
      for (auto piece_prom : (pos->side == Color::WHITE) ? WhitePromPiece : BlackPromPiece)
        move_push(move_list, encode_move(source_square, target_square, piece, piece_prom, 0, 0, 0, 0));            
      pop_bit(b3, target_square);
    }
  }

  // Standard and en passant captures
  b1 = shift<RIGHT>(shift<forward>(pawnsNotOn7)) & enemies;
  b2 = shift<LEFT>(shift<forward>(pawnsNotOn7)) & enemies;

  while (b1)
  {
    target_square = get_lsb_index(b1);
    source_square = shift<LEFT>(shift<backward>(target_square));
    move_push(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));            
    pop_bit(b1, target_square);
  }

  while (b2)
  {
    target_square = get_lsb_index(b2);
    source_square = shift<RIGHT>(shift<backward>(target_square));
    move_push(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));            
    pop_bit(b2, target_square);
  }

  if (pos->enpassant != Square::NO_SQUARE)
  {
    b1 = pawnsNotOn7 & get_pawn_attacks(pos->enpassant, ~pos->side); // symmetry of attacks
    while (b1)
    {
      source_square = get_lsb_index(b1);
      move_push(move_list, encode_move(source_square, pos->enpassant, piece, (Piece)0, 0, 0, 1, 0));            
      pop_bit(b1, source_square);
    }
  }
}

void generate_castle_moves(Position* pos, MoveList* move_list) {
  if (pos->side == Color::WHITE){
    if (pos->castle_rights & Castle_Right::WOO) {
      if (!get_bit(pos->occupancies[Color::BOTH], Square::F1) && 
          !get_bit(pos->occupancies[Color::BOTH], Square::G1) &&
          !is_square_attacked(pos, Square::E1, Color::BLACK)  &&
          !is_square_attacked(pos, Square::F1, Color::BLACK))
            move_push(move_list, encode_move(Square::E1, Square::G1, Piece::WK, (Piece)0, 0, 0, 0, 1));
    }
    if (pos->castle_rights & Castle_Right::WOOO) {
      if (!get_bit(pos->occupancies[Color::BOTH], Square::B1) && 
          !get_bit(pos->occupancies[Color::BOTH], Square::C1) &&
          !get_bit(pos->occupancies[Color::BOTH], Square::D1) &&
          !is_square_attacked(pos, Square::D1, Color::BLACK)  &&
          !is_square_attacked(pos, Square::E1, Color::BLACK))
            move_push(move_list, encode_move(Square::E1, Square::C1, Piece::WK, (Piece)0, 0, 0, 0, 1));
    }
  }
  else {
    if (pos->castle_rights & Castle_Right::BOO) {
      if (!get_bit(pos->occupancies[Color::BOTH], Square::F8) && 
          !get_bit(pos->occupancies[Color::BOTH], Square::G8) &&
          !is_square_attacked(pos, Square::E8, Color::WHITE)  &&
          !is_square_attacked(pos, Square::F8, Color::WHITE))
            move_push(move_list, encode_move(Square::E8, Square::G8, Piece::BK, (Piece)0, 0, 0, 0, 1));
    }
    if (pos->castle_rights & Castle_Right::BOOO) {
      if (!get_bit(pos->occupancies[Color::BOTH], Square::B8) &&
          !get_bit(pos->occupancies[Color::BOTH], Square::C8) && 
          !get_bit(pos->occupancies[Color::BOTH], Square::D8) &&
          !is_square_attacked(pos, Square::D8, Color::WHITE)  &&
          !is_square_attacked(pos, Square::E8, Color::WHITE))
            move_push(move_list, encode_move(Square::E8, Square::C8, Piece::BK, (Piece)0, 0, 0, 0, 1));
    }
  }
}

void move_push(MoveList* move_list, Move move) {
  move_list->moves[move_list->move_count].move = move;
  move_list->moves[move_list->move_count].score = 0;
  move_list->move_count++;
}

void print_move_list(MoveList* move_list) {
  std::cout << "  move    piece   double   enpassant   castling\n\n";
  for (int count = 0; count < move_list->move_count; count++)
  {
    Move move = move_list->moves[count].move;

    Square source_square = get_move_source(move);
    Square target_square = get_move_target(move);
    Piece piece = get_move_piece(move);
    Piece promoted = get_move_promoted(move);
    int capture_f = get_move_capture_f(move);
    int double_f = get_move_double_f(move);
    int enpassant_f = get_move_enpassant_f(move);
    int castling_f = get_move_castling_f(move);

      if (capture_f)
        std::cout << "  " << square_to_coordinates[source_square] << "x" << square_to_coordinates[target_square] << promoted_pieces.at(promoted) << "   " << ascii_pieces[piece] << "       " << double_f << "        " << enpassant_f << "           " << castling_f << '\n';
      else
        std::cout << "  " << square_to_coordinates[source_square] << square_to_coordinates[target_square] << promoted_pieces.at(promoted) << "   " << ascii_pieces[piece] << "       " << double_f << "        " << enpassant_f << "           " << castling_f << '\n';
  
  }
  std::cout << "\n\n" << "  Total number of moves: " << move_list->move_count << '\n' << std::endl;
}