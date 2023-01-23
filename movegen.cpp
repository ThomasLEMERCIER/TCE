#include "movegen.hpp"

#include <stdio.h>

void generate_moves(Position* pos, MoveList* move_list) {
  move_list->move_count = 0;

  generate_castle_moves(pos, move_list);
  if (pos->side == Color::white) {
    generate_pawn_moves<Color::white>(pos, move_list);
    generate_piece_moves<Piece::N>(pos, move_list);
    generate_piece_moves<Piece::B>(pos, move_list);
    generate_piece_moves<Piece::R>(pos, move_list);
    generate_piece_moves<Piece::Q>(pos, move_list);
    generate_piece_moves<Piece::K>(pos, move_list);
  } else {
    generate_pawn_moves<Color::black>(pos, move_list);
    generate_piece_moves<Piece::n>(pos, move_list);
    generate_piece_moves<Piece::b>(pos, move_list);
    generate_piece_moves<Piece::r>(pos, move_list);
    generate_piece_moves<Piece::q>(pos, move_list);
    generate_piece_moves<Piece::k>(pos, move_list);
  }
}

template<Piece piece>
void generate_piece_moves(Position* pos, MoveList* move_list) {
  Bitboard bb = pos->bitboards[piece];
  Bitboard attacks;
  Square source_square, target_square;

  while (bb) {
    source_square = get_lsb_index(bb);
    attacks = get_attacks_bb<piece>(source_square, pos->occupancies[Color::both]) & ((pos->side == Color::white) ? ~pos->occupancies[Color::white] : ~pos->occupancies[Color::black]);

    while (attacks)
    {
      target_square = get_lsb_index(attacks);

      if (!get_bit((pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white], target_square))
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
  constexpr Piece piece = (c == Color::white) ? Piece::P : Piece::p;
  constexpr Direction forward  = (c == Color::white) ? Direction::UP : Direction::DOWN;
  constexpr Direction backward  = (c == Color::white) ? Direction::DOWN : Direction::UP;
  constexpr Bitboard RelativeRank7 = (c == Color::white) ? RANK_7_BB : RANK_2_BB;
  constexpr Bitboard RelativeRank3 = (c == Color::white) ? RANK_3_BB : RANK_6_BB;

  Bitboard pawns = pos->bitboards[piece];
  Bitboard pawnsOn7 = pawns & RelativeRank7;
  Bitboard pawnsNotOn7 = pawns & ~RelativeRank7;

  Bitboard emptySquares = ~pos->occupancies[Color::both];
  Bitboard enemies = (pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white];

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
      for (auto piece_prom : (pos->side == Color::white) ? WhitePromPiece : BlackPromPiece)
        move_push(move_list, encode_move(source_square, target_square, piece, piece_prom, 1, 0, 0, 0));            
      pop_bit(b1, target_square);
    }

    while (b2) {
      target_square = get_lsb_index(b2);
      source_square = shift<RIGHT>(shift<backward>(target_square));
      for (auto piece_prom : (pos->side == Color::white) ? WhitePromPiece : BlackPromPiece)
        move_push(move_list, encode_move(source_square, target_square, piece, piece_prom, 1, 0, 0, 0));            
      pop_bit(b2, target_square);
    }

    while (b3) {
      target_square = get_lsb_index(b3);
      source_square = shift<backward>(target_square);
      for (auto piece_prom : (pos->side == Color::white) ? WhitePromPiece : BlackPromPiece)
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

  if (pos->enpassant != Square::no_sq)
  {
    b1 = pawnsNotOn7 & get_pawn_attacks(pos->enpassant, ~pos->side); // symetry of attacks
    while (b1)
    {
      source_square = get_lsb_index(b1);
      move_push(move_list, encode_move(source_square, pos->enpassant, piece, (Piece)0, 0, 0, 1, 0));            
      pop_bit(b1, source_square);
    }
  }
}

void generate_castle_moves(Position* pos, MoveList* move_list) {
  if (pos->side == Color::white){
    if (pos->castle_rights & Castle_Right::wk) {
      if (!get_bit(pos->occupancies[Color::both], Square::f1) && 
          !get_bit(pos->occupancies[Color::both], Square::g1) &&
          !is_square_attacked(pos, Square::e1, Color::black)  &&
          !is_square_attacked(pos, Square::f1, Color::black))
            move_push(move_list, encode_move(Square::e1, Square::g1, Piece::K, (Piece)0, 0, 0, 0, 1));
    }
    if (pos->castle_rights & Castle_Right::wq) {
      if (!get_bit(pos->occupancies[Color::both], Square::b1) && 
          !get_bit(pos->occupancies[Color::both], Square::c1) &&
          !get_bit(pos->occupancies[Color::both], Square::d1) &&
          !is_square_attacked(pos, Square::d1, Color::black)  &&
          !is_square_attacked(pos, Square::e1, Color::black))
            move_push(move_list, encode_move(Square::e1, Square::c1, Piece::K, (Piece)0, 0, 0, 0, 1));
    }
  }
  else {
    if (pos->castle_rights & Castle_Right::bk) {
      if (!get_bit(pos->occupancies[Color::both], Square::f8) && 
          !get_bit(pos->occupancies[Color::both], Square::g8) &&
          !is_square_attacked(pos, Square::e8, Color::white)  &&
          !is_square_attacked(pos, Square::f8, Color::white))
            move_push(move_list, encode_move(Square::e8, Square::g8, Piece::k, (Piece)0, 0, 0, 0, 1));
    }
    if (pos->castle_rights & Castle_Right::bq) {
      if (!get_bit(pos->occupancies[Color::both], Square::b8) &&
          !get_bit(pos->occupancies[Color::both], Square::c8) && 
          !get_bit(pos->occupancies[Color::both], Square::d8) &&
          !is_square_attacked(pos, Square::d8, Color::white)  &&
          !is_square_attacked(pos, Square::e8, Color::white))
            move_push(move_list, encode_move(Square::e8, Square::c8, Piece::k, (Piece)0, 0, 0, 0, 1));
    }
  }
}

void move_push(MoveList* move_list, Move move) {
  move_list->moves[move_list->move_count].move = move;
  move_list->moves[move_list->move_count].score = 0;
  move_list->move_count++;
}

void print_move_list(MoveList* move_list) {
  printf("  move    piece   double   enpassant   castling\n\n");
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
        printf("  %sx%s%c  %c       %d        %d           %d\n", square_to_coordinates[source_square],
                                                                  square_to_coordinates[target_square],
                                                                  promoted_pieces.at(promoted),
                                                                  ascii_pieces[piece],
                                                                  (double_f) ? 1 : 0,
                                                                  (enpassant_f) ? 1 : 0,
                                                                  (castling_f) ? 1 : 0);
      else
        printf("  %s%s%c   %c       %d        %d           %d\n", square_to_coordinates[source_square],
                                                                  square_to_coordinates[target_square],
                                                                  promoted_pieces.at(promoted),
                                                                  ascii_pieces[piece],
                                                                  (double_f) ? 1 : 0,
                                                                  (enpassant_f) ? 1 : 0,
                                                                  (castling_f) ? 1 : 0);
  
  }
  printf("\n\n Total number of moves: %d\n\n", move_list->move_count);
}