#include "evaluate.hpp"

#include "bitboard.hpp"

Bitboard isolated_masks[SQUARE_NB];
Bitboard white_passed_masks[SQUARE_NB];
Bitboard black_passed_masks[SQUARE_NB];

void init_evaluation_masks() {
  for (Rank rank = Rank::RANK_8; rank <= Rank::RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= File::FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (file > File::FILE_A)
        isolated_masks[square] |= get_file_bb(shift<Direction::LEFT>(file));
      if (file < File::FILE_H)
        isolated_masks[square] |= get_file_bb(shift<Direction::RIGHT>(file));
    }
  }
    
  for (Rank rank = Rank::RANK_8; rank <= Rank::RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= File::FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (file > File::FILE_A)
        white_passed_masks[square] |= get_file_bb(shift<Direction::LEFT>(file));
      white_passed_masks[square] |= get_file_bb(file);
      if (file < File::FILE_H)
        white_passed_masks[square] |= get_file_bb(shift<Direction::RIGHT>(file));

      for (Rank r = Rank::RANK_1; r >= rank; --r) 
        white_passed_masks[square] &= ~get_rank_bb(r);
    }
  }
    
  for (Rank rank = Rank::RANK_8; rank <= Rank::RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= File::FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (file > File::FILE_A)
        black_passed_masks[square] |= get_file_bb(shift<Direction::LEFT>(file));
      black_passed_masks[square] |= get_file_bb(file);
      if (file < File::FILE_H)
        black_passed_masks[square] |= get_file_bb(shift<Direction::RIGHT>(file));

      for (Rank r = Rank::RANK_8; r <= rank; ++r) 
        black_passed_masks[square] &= ~get_rank_bb(r);
    }
  }
}

Score evaluate(Position* pos) {
  Score score = 0;

  U64 bitboard;
  Square square;
  int double_pawns = 0;
  
  for (Piece piece = Piece::WP; piece <= Piece::BK; ++piece) {
    bitboard = pos->bitboards[piece];

    while (bitboard)
    {
      
      square = get_lsb_index(bitboard);
      score += material_score[piece];

      switch (piece)
      {
        case Piece::WP:
          score += pawn_score[square];

          double_pawns = count_bits(bitboard & get_file_bb(get_file(square)));
          
          if (double_pawns > 1)
            score += double_pawn_penalty;

          if ((pos->bitboards[Piece::WP] & isolated_masks[square]) == 0)
            score += isolated_pawn_penalty;

          if ((white_passed_masks[square] & pos->bitboards[Piece::BP]) == 0)
            score += passed_pawn_bonus[get_rank(square)];
          
          break;

        case Piece::WN: score += knight_score[square]; break;

        case Piece::WB:
          score += bishop_score[square];
          score += count_bits(get_bishop_attacks(square, pos->occupancies[Color::BOTH]));   
          break;

       case Piece::WR:
          score += rook_score[square];

          if ((pos->bitboards[Piece::WP] & get_file_bb(get_file(square))) == 0)
            score += semi_open_file_score;

          if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
            score += open_file_score;
          break;
        
        case Piece::WQ:
          score += count_bits(get_queen_attacks(square, pos->occupancies[Color::BOTH]));
          break;
        
        case Piece::WK:
          score += king_score[square];

          if ((pos->bitboards[Piece::WP] & get_file_bb(get_file(square))) == 0)
            score -= semi_open_file_score;

          if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
            score -= open_file_score;

          score += count_bits(get_king_attacks(square) & pos->occupancies[Color::WHITE]) * king_shield_bonus;
          break;

        case Piece::BP:
          score -= pawn_score[mirror_score[square]];

          double_pawns = count_bits(bitboard & get_file_bb(get_file(square)));       

          if (double_pawns > 1)
            score -= double_pawn_penalty;

          if ((pos->bitboards[Piece::BP] & isolated_masks[square]) == 0)
            score -= isolated_pawn_penalty;

          if ((black_passed_masks[square] & pos->bitboards[Piece::WP]) == 0)
            score -= passed_pawn_bonus[get_rank(mirror_score[square])];

          break;

        case Piece::BN: score -= knight_score[mirror_score[square]]; break;
        
        case Piece::BB:
          score -= bishop_score[mirror_score[square]];

          score -= count_bits(get_bishop_attacks(square, pos->occupancies[Color::BOTH]));
          break;

        case Piece::BR:
          score -= rook_score[mirror_score[square]];

          if ((pos->bitboards[Piece::BP] & get_file_bb(get_file(square))) == 0)
            score -= semi_open_file_score;

          if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
            score -= open_file_score;
          
          break;
        
        case Piece::BQ:
          score -= count_bits(get_queen_attacks(square, pos->occupancies[Color::BOTH]));
          break;
        
        case Piece::BK:
          score -= king_score[mirror_score[square]];

          if ((pos->bitboards[Piece::BP] & get_file_bb(get_file(square))) == 0)
              score += semi_open_file_score;

          if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
              score += open_file_score;

          score -= count_bits(get_king_attacks(square) & pos->occupancies[Color::BLACK]) * king_shield_bonus;
          break;

        default:
          break;
      }

      // pop ls1b
      pop_bit(bitboard, square);
    }
  }
  // return final evaluation based on side
  return (pos->side == Color::WHITE) ? score : -score;
}