#include "search.hpp"

#include "bitboard.hpp"
#include "movegen.hpp"
#include "orderer.hpp"
#include "evaluate.hpp"
#include "ttable.hpp"
#include "timeman.hpp"

#include <thread>
#include <cstring>    // memset
#include <iostream>
#include <algorithm>  // std::min

std::thread search_thread;
ThreadData search_td;
std::atomic<bool> search_stopped;

inline void check_time(const SearchLimits& limits) {
  if (search_stopped) return;
  if (!limits.use_time_management()) return;
  if (tm.is_time_up()) {
    search_stopped = true;
  }
}
  
void start_search(const Position& pos, int depth, const SearchLimits& limits) {
  
  // stop previous search
  stop_search();

  // ====== reset search data ======  
  search_td.thread_id = 0;
  search_td.depth = depth;
  search_td.pos = pos;

  search_td.nodes = 0;

  memset(search_td.killer_moves, 0, sizeof(KillerMoves));
  memset(search_td.history_moves, 0, sizeof(HistoryMoves));
  memset(search_td.pv_length, 0, sizeof(search_td.pv_length));
  memset(search_td.pv_table, 0, sizeof(search_td.pv_table));

  search_td.limits = limits;
  // ===============================

  // start new search
  search_stopped = false;
  search_thread = std::thread(search_position, std::ref(search_td));
}

void stop_search() {
  search_stopped = true;
  if (search_thread.joinable()) search_thread.join();
}

Score quiescence(Position* pos, Score alpha, Score beta, ThreadData& td) {
  // check if time is up
  if((td.thread_id == 0) && ((td.nodes & check_every_nodes ) == 0))
    check_time(td.limits);

  // initialize TT entry
  TTEntry tte;
  Move previous_best_move = UNDEFINED_MOVE;
  Score score;
  
  // probe TT
  if (TT.probe(pos, tte)) {
    if (tte.flag == ExactFlag) {
      score = tte.score;

      if (score < - MATE_IN_MAX_PLY) score += pos->ply;
      if (score > MATE_IN_MAX_PLY) score -= pos->ply;
      return score;
    }
    if ((tte.flag == UpperBound) &&  tte.score <= alpha) {
      return alpha;
    }
    if ((tte.flag == LowerBound) &&  tte.score >= beta) {
      return beta;
    }
    previous_best_move = tte.best_move;
  }

  Score evaluation = evaluate(pos);

  // fail-hard beta cutoff
  if (evaluation >= beta) {
    return beta;
  }

  // if we search too deep, return static evaluation
  if (pos->ply > MAX_PLY_SEARCH - 1)
    return evaluation;

  // update alpha if static evaluation is higher
  if (evaluation > alpha) {
    alpha = evaluation;
  }

  // generate all moves
  Orderer orderer = Orderer(pos, &td.killer_moves, &td.history_moves, previous_best_move);
  Move current_move;

  while ((current_move = orderer.next_move()) != UNDEFINED_MOVE) {
    Position next_pos = Position(pos);

    // check if move is legal
    if (!make_move(&next_pos, current_move, Move_Type::ONLY_CAPTURES))  {
      continue;
    }

    // update node count
    td.nodes++;
  
    // recursive call
    score = -quiescence(&next_pos, -beta, -alpha, td);

    // if the search was stopped, return NO_VALUE
    if (search_stopped) return NO_VALUE;
    
    // update alpha if score is higher
    if (score > alpha) {
      alpha = score;
      // fail-hard beta cutoff
      if (score >= beta) {
        return beta;
      }
    }

  }
  return alpha;
}

Score negamax(Position* pos, Score alpha, Score beta, int depth, bool null_pruning, ThreadData& td) {
  // check if time is up
  if((td.thread_id == 0) && ((td.nodes & check_every_nodes ) == 0))
    check_time(td.limits);

  // check for draw
  if (pos->is_repetition()) return DRAW_VALUE;

  // previous alpha value and PV node flag
  int pv_node = (beta - alpha) > 1;
  Score original_alpha = alpha;

  // initialize TT entry
  TTEntry tte;
  Move previous_best_move = UNDEFINED_MOVE;
  Score score;

  // probe TT
  if (TT.probe(pos, tte)) {
    if (pos->ply && tte.depth >= depth && !pv_node) {
      if (tte.flag == ExactFlag) {
        score = tte.score;

        if (score < - MATE_IN_MAX_PLY) score += pos->ply;
        if (score > MATE_IN_MAX_PLY) score -= pos->ply;
        return score;
      }
      if ((tte.flag == UpperBound) &&  tte.score <= alpha) {
        return alpha;
      }
      if ((tte.flag == LowerBound) &&  tte.score >= beta) {
        return beta;
      }
    }
    previous_best_move = tte.best_move;
  }

  // adjust depth of the the principal variation
  td.pv_length[pos->ply] = pos->ply;

  // check if king is in check
  bool in_check = is_square_attacked(pos, (pos->side == Color::WHITE) ? get_lsb_index(pos->bitboards[Piece::WK]) :
                                                                get_lsb_index(pos->bitboards[Piece::BK]),
                                                                ~pos->side);

  // increase search depth if king has been exposed to check
  if (in_check)
    depth++;

  // check if we reached the maximum depth and return the quiescence search
  if (depth == 0)
    return quiescence(pos, alpha, beta, td);

  // static evaluation
  if (pos->ply > MAX_PLY_SEARCH - 1)
    return evaluate(pos);

  // null move pruning
  if (null_pruning && pos->ply && depth > null_move_reduction && !pv_node && !in_check) {
    Position next_pos = Position(pos);
    make_null_move(&next_pos);

    score = -negamax(&next_pos, -beta, -beta + 1, depth - null_move_reduction, false, td);

    if (score >= beta) {
      return beta;
    }

  }

  // generate all moves
  Orderer orderer = Orderer(pos, &td.killer_moves, &td.history_moves, previous_best_move);
  Move best_move = UNDEFINED_MOVE;

  Score best_score = -INF;
  int legal_moves = 0;
  int moves_searched = 0;
  Move current_move;

  bool lmr = !in_check && depth > lmr_reduction;

  while ((current_move = orderer.next_move()) != UNDEFINED_MOVE) {
    Position next_pos = Position(pos);

    // check if move is legal
    if (!make_move(&next_pos, current_move, Move_Type::ALL_MOVES)) {
      continue;
    }

    // update node counts
    td.nodes++;
    legal_moves++;

    // full depth search
    if (moves_searched == 0) score = -negamax(&next_pos, -beta, -alpha, depth - 1, true, td); // full depth search
    // late move reduction (LMR)
    else {
      // condition to consider LMR
      if(lmr && (moves_searched >= full_depth_moves) && (get_move_capture_f(current_move) == 0) && (get_move_promoted(current_move) == 0)) {
        // search current move with reduced depth:
        score = -negamax(&next_pos, -alpha - 1, -alpha, depth - lmr_reduction, true, td);
      }
      else score = alpha + 1; // fail-soft to trigger PVS

      // Principal Variation Search (PVS)
      if(score > alpha)
      {
        score = -negamax(&next_pos, -alpha - 1, -alpha, depth-1, true, td);
  
      if((score > alpha) && (score < beta))
        score = -negamax(&next_pos, -beta, -alpha, depth-1, true, td);
      }
    }

    // update number of moves searched
    moves_searched++;

    // if the search was stopped, return NO_VALUE
    if (search_stopped) return NO_VALUE;

    if (score > best_score) {

      best_score = score;
      best_move = current_move;

      if (score > alpha) {
        
        // store history moves
        if (!get_move_capture_f(current_move))
          td.history_moves[get_move_piece(current_move)][get_move_target(current_move)] += depth;

        alpha = score;

        // update principal variation table
        td.pv_table[pos->ply][pos->ply] = current_move;
        for (int next_ply = pos->ply + 1; next_ply < td.pv_length[pos->ply + 1]; next_ply++)
          td.pv_table[pos->ply][next_ply] = td.pv_table[pos->ply + 1][next_ply];
        td.pv_length[pos->ply] = td.pv_length[pos->ply + 1];

        // fail-hard beta cutoff
        if (score >= beta) {

          // store killer moves
          if (!get_move_capture_f(current_move)) {
            td.killer_moves[1][pos->ply] = td.killer_moves[0][pos->ply];
            td.killer_moves[0][pos->ply] = current_move;
          }
          break;
        }
      }
    }
  }

  // if there are no legal moves, check if we are in check or not
  if (legal_moves == 0) {
    // if we are in check, return mate score
    if (in_check) {
      return - (MATE_VALUE - pos->ply); // number of move to get to mate (fav. faster mate)
    }
    // otherwise return draw score
    else {
      return DRAW_VALUE;
    }
  }

  // update TT entry
  TTFlag tt_flag = (best_score >= beta) ? LowerBound : (alpha > original_alpha) ? ExactFlag : UpperBound;
  Score return_score = (best_score >= beta) ? beta : (alpha > original_alpha) ? best_score  : alpha;
  TT.write_entry(pos, tt_flag, return_score, depth, best_move);

  return return_score;
}

Score aspiration_window(Position* pos, Score previous_score, int depth, ThreadData& td) {

  Score alpha = -INF;
  Score beta = INF;

  Score window = aspiration_window_start - aspiration_window_decrement * (depth - aspiration_window_depth);
  window = std::max(window, aspiration_window_end);

  // apply aspiration window if previous score is not NO_VALUE and depth is enough
  if (previous_score != NO_VALUE && depth > aspiration_window_depth) {
    alpha = previous_score - window;
    beta = previous_score + window;
  }

  Score score;

  for (;;) {
    score = negamax(pos, alpha, beta, depth, false, td);

    // if the search was stopped, return NO_VALUE
    if (search_stopped) return NO_VALUE;

    // out of bounds
    if (score <= alpha) {
      beta = (alpha + beta + 1) / 2;
      alpha = alpha - window;
    }
    else if (score >= beta) {
      beta = beta + window;
    }
    // within bounds
    else {
      break;
    }

    // increase window if it is too small
    window += window / 2;
    if (window > aspiration_window_max) {
      alpha = -INF;
      beta = INF;
    }    
  }
  
  return score;
}

void search_position(ThreadData& td) {
  // start timer
  TimePoint top_time = get_time_ms();

  // extract position
  Position* pos = &td.pos;
  pos->ply = 0;

  Move bestmove = UNDEFINED_MOVE;
  Score score = NO_VALUE;


  // iterative deepening
  for (int current_depth = 1; current_depth <= td.depth; current_depth++) {
    score = aspiration_window(pos, score, current_depth, td);

    // if the search was stopped, break
    if (search_stopped) break;

    // retrieve best move
    bestmove = td.pv_table[0][0];
    
    // print info on the main thread
    if (td.thread_id == 0) {
      if (score > -MATE_VALUE && score < -MATE_IN_MAX_PLY) {
        std::cout << "info score mate " << -(score + MATE_VALUE) / 2 - 1 << " depth " << current_depth << " nodes " << td.nodes << " time " << get_time_ms() - top_time << " pv ";
      }
      else if (score > MATE_IN_MAX_PLY && score < MATE_VALUE) {
        std::cout << "info score mate " << (MATE_VALUE - score) / 2 + 1 << " depth " << current_depth << " nodes " << td.nodes << " time " << get_time_ms() - top_time << " pv ";
      }  
      else
        std::cout << "info score cp " << score << " depth " << current_depth << " nodes " << td.nodes << " time " << get_time_ms() - top_time << " pv ";

      for (int count = 0; count < td.pv_length[0]; count++) {
        print_move(td.pv_table[0][count]);
        std::cout << " ";
      }
      std::cout << '\n';
      std::cout << std::flush;
    }

    // if the score is mate, break
    if ((score > -MATE_VALUE && score < -MATE_IN_MAX_PLY) || (score > MATE_IN_MAX_PLY && score < MATE_VALUE))
      break;
  }

  // print best move
  if (td.thread_id == 0) {
    std::cout << "bestmove ";
    print_move(bestmove);
    std::cout << '\n';
    std::cout << std::flush;
  }
}

