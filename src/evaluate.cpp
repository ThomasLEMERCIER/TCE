#include "evaluate.hpp"

#include "bitboard.hpp"

#include <algorithm> // std::clamp
#ifdef TRACE_EVAL
#include <iostream>
#endif

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
  Phase phase = 0;
  ScoreExt score = { 0, 0 };

#ifdef TRACE_EVAL
  std::cout << "=========EVALUATION=========" << std::endl;
  print_board(pos);
#endif

  score += evaluate_material(pos, phase);
  score += evaluate_pieces<Piece::WN>(pos);
  score += evaluate_pieces<Piece::WB>(pos);
  score += evaluate_pieces<Piece::WR>(pos);
  score += evaluate_pieces<Piece::WQ>(pos);
  score += evaluate_pawns(pos);
  score += evaluate_kings(pos);

  Score final_score = (score.mg * phase + score.eg * (PhaseMidGame - phase)) / PhaseMidGame;

#ifdef TRACE_EVAL
  std::cout << "Final score: "; PRINT_SCORE(score); std::cout << std::endl;
  std::cout << "Final score (phase): " << final_score << std::endl;
  std::cout << "=========END EVALUATION=========" << std::endl;
#endif

  return (pos->side == Color::WHITE) ? final_score : -final_score;
}

ScoreExt evaluate_material(Position* pos, Phase& phase) {
  ScoreExt score = { 0, 0 };

  // extract number of pieces for each side
  int white_pawns = pos->piece_count[Piece::WP];
  int white_knights = pos->piece_count[Piece::WN];
  int white_bishops = pos->piece_count[Piece::WB];
  int white_rooks = pos->piece_count[Piece::WR];
  int white_queens = pos->piece_count[Piece::WQ];

  int black_pawns = pos->piece_count[Piece::BP];
  int black_knights = pos->piece_count[Piece::BN];
  int black_bishops = pos->piece_count[Piece::BB];
  int black_rooks = pos->piece_count[Piece::BR];
  int black_queens = pos->piece_count[Piece::BQ];
  
  // compute material score
  score += (white_pawns - black_pawns) * PawnValue;
  score += (white_knights - black_knights) * KnightValue;
  score += (white_bishops - black_bishops) * BishopValue;
  score += (white_rooks - black_rooks) * RookValue;
  score += (white_queens - black_queens) * QueenValue;

  // compute bonuses and penalties
  if (white_bishops > 1) score += BishopPairBonus;
  if (black_bishops > 1) score -= BishopPairBonus;
  
  if (white_knights > 1) score += KnightPairPenalty;
  if (black_knights > 1) score -= KnightPairPenalty;

  if (white_rooks > 1) score += RookPairPenalty;
  if (black_rooks > 1) score -= RookPairPenalty;

  if (white_pawns == 0) score += NoPawnPenalty;
  if (black_pawns == 0) score -= NoPawnPenalty;

  // compute phase
  phase += (white_knights + black_knights) * KnightValue.mg;
  phase += (white_bishops + black_bishops) * BishopValue.mg;
  phase += (white_rooks + black_rooks) * RookValue.mg;
  phase += (white_queens + black_queens) * QueenValue.mg;

  phase = std::clamp(phase, EndGameThreshold, MidGameThreshold);
  phase = (phase - EndGameThreshold) * PhaseMidGame / (MidGameThreshold - EndGameThreshold);

#ifdef TRACE_EVAL
  std::cout << "Material score: "; PRINT_SCORE(score); std::cout << std::endl;
  std::cout << "Phase: " << phase << std::endl;
#endif

  return score;
}

template <Piece piece>
ScoreExt evaluate_pieces(Position* pos) {
  constexpr Piece mirror_piece = ~piece;

  ScoreExt score = { 0, 0 };

  Bitboard bitboard;
  Square square;
  int mobility;

  // evaluate white pieces
  bitboard = pos->bitboards[piece];
  while (bitboard) {
    square = get_lsb_index(bitboard);

    // compute mobility for bishop, rook and queen
    if constexpr (piece == Piece::WB || piece == Piece::WR || piece == Piece::WQ)
      mobility = count_bits(get_attacks_bb<piece>(square, pos->occupancies[Color::BOTH]));
    else
      mobility = 0;

    // compute rook on open file bonus
    if constexpr (piece == Piece::WR) {    
      if ((pos->bitboards[Piece::WP] & get_file_bb(get_file(square))) == 0)
        score += semi_open_file_score;

      if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
        score += open_file_score;
    }

    score += piece_square_tables[piece][square];
    score += mobility * mobility_score[piece];

    pop_bit(bitboard, square);
  }

  // evaluate black pieces
  bitboard = pos->bitboards[mirror_piece];
  while (bitboard) {
    square = get_lsb_index(bitboard);

    // compute mobility for bishop, rook and queen
    if constexpr (piece == Piece::WB || piece == Piece::WR || piece == Piece::WQ)
      mobility = count_bits(get_attacks_bb<mirror_piece>(square, pos->occupancies[Color::BOTH]));
    else
      mobility = 0;

    // compute rook on open file bonus
    if constexpr (piece == Piece::WR) {    
      if ((pos->bitboards[Piece::BP] & get_file_bb(get_file(square))) == 0)
        score -= semi_open_file_score;

      if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
        score -= open_file_score;
    }

    score -= piece_square_tables[piece][mirror_square[square]];
    score -= mobility * mobility_score[piece];

    pop_bit(bitboard, square);
  }

#ifdef TRACE_EVAL
  std::cout << "Piece score " << "(" << ascii_pieces[piece] << "): "; PRINT_SCORE(score); std::cout << std::endl;
#endif

  return score;
}

ScoreExt evaluate_kings(Position* pos) {
  ScoreExt score = { 0, 0 };

  Bitboard bitboard;
  Square square;

  // evaluate white king
  bitboard = pos->bitboards[Piece::WK];
  square = get_lsb_index(bitboard);

  score += king_table[square];

  // open king file penalty
  if ((pos->bitboards[Piece::WP] & get_file_bb(get_file(square))) == 0)
    score -= semi_open_file_score;

  if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
    score -= open_file_score;


  // king shield bonus
  score += count_bits(get_king_attacks(square) & pos->occupancies[Color::WHITE]) * king_shield_bonus;


  // evaluate black king
  bitboard = pos->bitboards[Piece::BK];
  square = get_lsb_index(bitboard);

  score -= king_table[mirror_square[square]];

  // open king file penalty
  if ((pos->bitboards[Piece::BP] & get_file_bb(get_file(square))) == 0)
    score += semi_open_file_score;

  if (((pos->bitboards[Piece::WP] | pos->bitboards[Piece::BP]) & get_file_bb(get_file(square))) == 0)
    score += open_file_score;

  // king shield bonus
  score -= count_bits(get_king_attacks(square) & pos->occupancies[Color::BLACK]) * king_shield_bonus;

#ifdef TRACE_EVAL
  std::cout << "King score: "; PRINT_SCORE(score); std::cout << std::endl;
#endif

  return score;
}

ScoreExt evaluate_pawns(Position* pos) {
  ScoreExt score = { 0, 0 };

  Bitboard bitboard;
  Square square;
  int double_pawns;


  // evaluate white pawns
  bitboard = pos->bitboards[Piece::WP];
  while (bitboard) {
    square = get_lsb_index(bitboard);

    score += pawn_table[square];


    double_pawns = count_bits(bitboard & get_file_bb(get_file(square)));
    if (double_pawns > 1)
      score += double_pawn_penalty;

    if ((pos->bitboards[Piece::WP] & isolated_masks[square]) == 0)
      score += isolated_pawn_penalty;

    if ((white_passed_masks[square] & pos->bitboards[Piece::BP]) == 0)
      score += passed_pawn_bonus[get_rank(square)];

    pop_bit(bitboard, square);
  }

  // evaluate black pawns
  bitboard = pos->bitboards[Piece::BP];
  while (bitboard) {
    square = get_lsb_index(bitboard);

    score -= pawn_table[mirror_square[square]];

    double_pawns = count_bits(bitboard & get_file_bb(get_file(square)));
    if (double_pawns > 1)
      score -= double_pawn_penalty;

    if ((pos->bitboards[Piece::BP] & isolated_masks[square]) == 0)
      score -= isolated_pawn_penalty;

    if ((black_passed_masks[square] & pos->bitboards[Piece::WP]) == 0)
      score -= passed_pawn_bonus[get_rank(mirror_square[square])];

    pop_bit(bitboard, square);
  }

#ifdef TRACE_EVAL
  std::cout << "Pawn score: "; PRINT_SCORE(score); std::cout << std::endl;
#endif

  return score;
}
