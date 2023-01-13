#include "movegen.hpp"

#include <stdio.h>

void generate_moves(MoveList *move_list, Position* pos) {
  move_list->move_count = 0;

  Square source_square, target_square;
  Bitboard bitboard, attacks;

  // loop over all piece
  for (Piece piece = Piece::P; piece <= Piece::k; ++piece) {
    bitboard = pos->bitboards[piece];

    // generate white pawns & white king castling moves
    if (pos->side == Color::white) {
      if (piece == Piece::P) {
        while  (bitboard) {
          source_square = get_lsb_index(bitboard);
          target_square = shift<UP>(source_square);

          // quiet pawn move
          if (!(target_square < Square::a8) && !get_bit(pos->occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= Square::a7 && source_square <= Square::h7) {
              // addd move into a move list            
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
            }
            else {
              // one square pawn move
              add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));

              // double square pawn move
              target_square = (Square)(source_square - 16);
              if ((source_square >= Square::a2 && source_square <= Square::h2) && !get_bit(pos->occupancies[Color::both], target_square))
                add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 1, 0, 0));

            }
          }

          // init pawn attacks bitboard
          attacks = get_pawn_attacks(source_square, pos->side) & pos->occupancies[Color::black];

          // generate pawn captures
          while (attacks)
          {
            target_square = get_lsb_index(attacks);
            
            if (source_square >= Square::a7 && source_square <= Square::h7) {
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
            }
            else
              add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (pos->enpassant != Square::no_sq)
          {
              // lookup pawn attacks and bitwise AND with enpassant square (bit)
              Bitboard enpassant_attacks = get_pawn_attacks(source_square, pos->side) & (1ULL << pos->enpassant);
              
              // make sure enpassant capture available
              if (enpassant_attacks)
              {
                // init enpassant capture target square
                Square target_enpassant = get_lsb_index(enpassant_attacks);
                add_move(move_list, encode_move(source_square, target_enpassant, piece, (Piece)0, 1, 0, 1, 0));
              }
          }

          // pop lsb from piece bitboard copy
          pop_bit(bitboard, source_square);
        }
        // white king bitboard index
      }
      else if (piece == Piece::K) {
        // king side castling is available
        if (pos->castle_rights & Castle_Right::wk) {
          // make sure square betwwen king and king's rook are empty
          if (!get_bit(pos->occupancies[Color::both], Square::f1) && !get_bit(pos->occupancies[Color::both], Square::g1))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(pos, Square::e1, Color::black) && !is_square_attacked(pos, Square::f1, Color::black)) {
              add_move(move_list, encode_move(Square::e1, Square::g1, piece, (Piece)0, 0, 0, 0, 1));
            }
          }
        }
        if (pos->castle_rights & Castle_Right::wq) {
          // make sure square betwwen king and queen's rook are empty
          if (!get_bit(pos->occupancies[Color::both], Square::b1) && !get_bit(pos->occupancies[Color::both], Square::c1)& !get_bit(pos->occupancies[Color::both], Square::d1))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(pos, Square::d1, Color::black) && !is_square_attacked(pos, Square::e1, Color::black)) {
              add_move(move_list, encode_move(Square::e1, Square::c1, piece, (Piece)0, 0, 0, 0, 1));
            }
          }
        }
      }
    }
    else {
      // black pawn bitboard index
      if (piece == Piece::p) {
        // loop over black pawns
        while  (bitboard) {
          source_square = get_lsb_index(bitboard);
          target_square = shift<DOWN>(source_square);

          if (!(target_square > Square::h1) && !get_bit(pos->occupancies[Color::both], target_square)) {
            // pawn promotion
            if (source_square >= Square::a2 && source_square <= Square::h2) {
              // addd move into a move list
              add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
            }
            else {
              // one square pawn move
              add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));

              // double square pawn move
              target_square = Square(source_square + 16);
              if ((source_square >= Square::a7 && source_square <= Square::h7) && !get_bit(pos->occupancies[Color::both], target_square))
                add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 1, 0, 0));
            }
          }

          // init pawn attacks bitboard
          attacks = get_pawn_attacks(source_square, pos->side) & pos->occupancies[Color::white];

          // generate pawn captures
          while (attacks)
          {
            // init target square
            target_square = get_lsb_index(attacks);
            
            if (source_square >= Square::a2 && source_square <= Square::h2) {
              add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
            }
            else
              add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (pos->enpassant != Square::no_sq)
          {
              // lookup pawn attacks and bitwise AND with enpassant square (bit)
              Bitboard enpassant_attacks = get_pawn_attacks(source_square, pos->side) & (1ULL << pos->enpassant);
              
              // make sure enpassant capture available
              if (enpassant_attacks)
              {
                // init enpassant capture target square
                Square target_enpassant = get_lsb_index(enpassant_attacks);
                add_move(move_list, encode_move(source_square, target_enpassant, piece, (Piece)0, 1, 0, 1, 0));
              }
          }
          
          // pop lsb from piece bitboard copy
          pop_bit(bitboard, source_square);
        }
      }
      else if (piece == k) {
        // king side castling is available
        if (pos->castle_rights & Castle_Right::bk) {
          // make sure square betwwen king and king's rook are empty
          if (!get_bit(pos->occupancies[Color::both], Square::f8) && !get_bit(pos->occupancies[Color::both], Square::g8))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(pos, Square::e8, Color::white) && !is_square_attacked(pos, Square::f8, Color::white)) {
              add_move(move_list, encode_move(Square::e8, Square::g8, piece, (Piece)0, 0, 0, 0, 1));
            }
          }
        }
        if (pos->castle_rights & Castle_Right::bq) {
          // make sure square betwwen king and queen's rook are empty
          if (!get_bit(pos->occupancies[Color::both], Square::b8) && !get_bit(pos->occupancies[Color::both], Square::c8)& !get_bit(pos->occupancies[Color::both], Square::d8))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(pos, Square::d8, Color::white) && !is_square_attacked(pos, Square::e8, Color::white)) {
              add_move(move_list, encode_move(Square::e8, Square::c8, piece, (Piece)0, 0, 0, 0, 1));
            }
          }
        }
      }
    }

    // generate knight moves
    if ((pos->side == Color::white) ? piece == Piece::N : piece == Piece::n) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = get_knight_attacks(source_square) & ((pos->side == Color::white) ? ~pos->occupancies[Color::white] : ~pos->occupancies[Color::black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate bishop moves
    if ((pos->side == Color::white) ? piece == Piece::B : piece == Piece::b) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = get_bishop_attacks(source_square, pos->occupancies[both]) & ((pos->side == Color::white) ? ~pos->occupancies[Color::white] : ~pos->occupancies[Color::black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate rook moves
    if ((pos->side == Color::white) ? piece == Piece::R : piece == Piece::r) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = get_rook_attacks(source_square, pos->occupancies[Color::both]) & ((pos->side == Color::white) ? ~pos->occupancies[Color::white] : ~pos->occupancies[Color::black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate queen moves
    if ((pos->side == Color::white) ? piece == Piece::Q : piece == Piece::q) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        attacks = get_queen_attacks(source_square, pos->occupancies[Color::both]) & ((pos->side == Color::white) ? ~pos->occupancies[Color::white] : ~pos->occupancies[Color::black]);

        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate king moves
    if ((pos->side == Color::white) ? piece == Piece::K : piece == Piece::k) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        attacks = get_king_attacks(source_square) & ((pos->side == Color::white) ? ~pos->occupancies[Color::white] : ~pos->occupancies[Color::black]);

        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((pos->side == Color::white) ? pos->occupancies[Color::black] : pos->occupancies[Color::white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, (Piece)0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

  }
}

void add_move(MoveList* move_list, Move move) {
  move_list->moves[move_list->move_count] = move; move_list->move_count++;
}

void print_move_list(MoveList* move_list) {
  printf("  move    piece   double   enpassant   castling\n\n");
  for (int count = 0; count < move_list->move_count; count++)
  {
    Move move = move_list->moves[count];

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