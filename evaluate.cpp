#include "evaluate.hpp"

#include <stdio.h>

Bitboard isolated_masks[SQUARE_NB];
Bitboard white_passed_masks[SQUARE_NB];
Bitboard black_passed_masks[SQUARE_NB];

void init_evaluation_masks() {
  for (Rank rank = Rank::RANK_8; rank <= RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (file > FILE_A)
        isolated_masks[square] |= get_file_bb(shift<LEFT>(file));
      if (file < FILE_H)
        isolated_masks[square] |= get_file_bb(shift<RIGHT>(file));
    }
  }
    
  for (Rank rank = Rank::RANK_8; rank <= RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (file > FILE_A)
        white_passed_masks[square] |= get_file_bb(shift<LEFT>(file));
      white_passed_masks[square] |= get_file_bb(file);
      if (file < FILE_H)
        white_passed_masks[square] |= get_file_bb(shift<RIGHT>(file));

      for (Rank r = Rank::RANK_1; r >= rank; --r) 
        white_passed_masks[square] &= ~get_rank_bb(r);
    }
  }
    
  for (Rank rank = Rank::RANK_8; rank <= RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (file > FILE_A)
        black_passed_masks[square] |= get_file_bb(shift<LEFT>(file));
      black_passed_masks[square] |= get_file_bb(file);
      if (file < FILE_H)
        black_passed_masks[square] |= get_file_bb(shift<RIGHT>(file));

      for (Rank r = Rank::RANK_8; r <= rank; ++r) 
        black_passed_masks[square] &= ~get_rank_bb(r);
    }
  }
}

int evaluate(Position* pos) {
  int score = 0;

  U64 bitboard;
  Square square;
  int double_pawns = 0;
  
  for (Piece piece = Piece::P; piece <= Piece::k; ++piece) {
    bitboard = pos->bitboards[piece];

    while (bitboard)
    {
      
      square = get_lsb_index(bitboard);
      score += material_score[piece];

      switch (piece)
      {
        case P:
          score += pawn_score[square];

          double_pawns = count_bits(bitboard & get_file_bb(get_file(square)));
          
          if (double_pawns > 1)
            score += double_pawn_penalty;

          if ((pos->bitboards[Piece::P] & isolated_masks[square]) == 0)
            score += isolated_pawn_penalty;

          if ((white_passed_masks[square] & pos->bitboards[Piece::p]) == 0)
            score += passed_pawn_bonus[get_rank(square)];
          
          break;

        case N: score += knight_score[square]; break;

        case B:
          score += bishop_score[square];
          score += count_bits(get_bishop_attacks(square, pos->occupancies[Color::both]));   
          break;

       case R:
          score += rook_score[square];

          if ((pos->bitboards[Piece::P] & get_file_bb(get_file(square))) == 0)
            score += semi_open_file_score;

          if (((pos->bitboards[Piece::P] | pos->bitboards[Piece::p]) & get_file_bb(get_file(square))) == 0)
            score += open_file_score;
          break;
        
        case Q:
          score += count_bits(get_queen_attacks(square, pos->occupancies[Color::both]));
          break;
        
        case K:
          score += king_score[square];

          if ((pos->bitboards[Piece::P] & get_file_bb(get_file(square))) == 0)
            score -= semi_open_file_score;

          if (((pos->bitboards[Piece::P] | pos->bitboards[Piece::p]) & get_file_bb(get_file(square))) == 0)
            score -= open_file_score;

          score += count_bits(get_king_attacks(square) & pos->occupancies[Color::white]) * king_shield_bonus;
          break;

        case p:
          score -= pawn_score[mirror_score[square]];

          double_pawns = count_bits(bitboard & get_file_bb(get_file(square)));       

          if (double_pawns > 1)
            score -= double_pawn_penalty;

          if ((pos->bitboards[Piece::p] & isolated_masks[square]) == 0)
            score -= isolated_pawn_penalty;

          if ((black_passed_masks[square] & pos->bitboards[Piece::P]) == 0)
            score -= passed_pawn_bonus[get_rank(mirror_score[square])];

          break;

        case n: score -= knight_score[mirror_score[square]]; break;
        
        case b:
          score -= bishop_score[mirror_score[square]];

          score -= count_bits(get_bishop_attacks(square, pos->occupancies[Color::both]));
          break;

        case r:
          score -= rook_score[mirror_score[square]];

          if ((pos->bitboards[Piece::p] & get_file_bb(get_file(square))) == 0)
            score -= semi_open_file_score;

          if (((pos->bitboards[Piece::P] | pos->bitboards[Piece::p]) & get_file_bb(get_file(square))) == 0)
            score -= open_file_score;
          
          break;
        
        case q:
          score -= count_bits(get_queen_attacks(square, pos->occupancies[Color::both]));
          break;
        
        case k:
          score -= king_score[mirror_score[square]];

          if ((pos->bitboards[Piece::p] & get_file_bb(get_file(square))) == 0)
              score += semi_open_file_score;

          if (((pos->bitboards[Piece::P] | pos->bitboards[Piece::p]) & get_file_bb(get_file(square))) == 0)
              score += open_file_score;

          score -= count_bits(get_king_attacks(square) & pos->occupancies[Color::black]) * king_shield_bonus;
          break;
      }

      // pop ls1b
      pop_bit(bitboard, square);
    }
  }
  // return final evaluation based on side
  return (pos->side == white) ? score : -score;
}