#include "orderer.hpp"

#include "search.hpp"

Orderer::Orderer(Position* p, KillerMoves* km, HistoryMoves* hm, Move p_best_move) : pos(p), killer_moves(km), history_moves(hm), previous_best_move(p_best_move) {
  generate_moves(pos, &move_list);  
  score_moves();
  current_idx = 0;
}

void Orderer::score_moves() {
  MoveExt* moveExt;
  for (int count = 0; count < move_list.move_count; count++) {
    moveExt = &move_list.moves[count];
    if (moveExt->move ==  previous_best_move) {
      moveExt->score = BEST_SCORE_SORT;
    }
    else if (get_move_capture_f(moveExt->move)) {
      Piece target_piece = Piece::WP;

      Piece start_piece = (pos->side == Color::WHITE) ? Piece::BP : Piece::WP;
      Piece end_piece = (pos->side == Color::WHITE) ? Piece::BK : Piece::WK;

      for (Piece piece = start_piece; piece <= end_piece; ++piece) {
        if (get_bit(pos->bitboards[piece], get_move_target(moveExt->move))) {
          target_piece = piece;
          break;
        }
      }

      moveExt->score =  mvv_lva[get_move_piece(moveExt->move)][target_piece] + CAP_SCORE_SORT;
    }
    else {
      if ((*killer_moves)[0][pos->ply] == moveExt->move) {
        moveExt->score = FIRST_KILLER_SORT;
      }
      else if ((*killer_moves)[1][pos->ply] == moveExt->move) {
        moveExt->score = SECOND_KILLER_SORT;
      }
      else {
        moveExt->score = (*history_moves)[get_move_piece(moveExt->move)][get_move_target(moveExt->move)];
      }
    }
  }
}

Move Orderer::next_move() {
  if (current_idx >= move_list.move_count) {
    return UNDEFINED_MOVE;
  }

  int highest_idx = current_idx;

  for (int i = current_idx + 1; i < move_list.move_count; i++) {
    if (move_list.moves[i].score > move_list.moves[highest_idx].score)
      highest_idx = i;
  }

  MoveExt temp = move_list.moves[current_idx];
  move_list.moves[current_idx] = move_list.moves[highest_idx];
  move_list.moves[highest_idx] = temp;

  return move_list.moves[current_idx++].move;
}