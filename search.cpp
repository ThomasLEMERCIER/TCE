#include "search.hpp"

#include <stdio.h>

SearchData sd;
TranspositionTable TT;

void TranspositionTable::clear() {
  memset(this, 0, sizeof(TranspositionTable));
}

void clear_search_data() {
  memset(&sd, 0, sizeof(SearchData));
}

int TranspositionTable::probe(Position* pos, TTEntry& tte) {
  tte = table[pos->hash_key % hash_size];

  if (tte.key == pos->hash_key)
    return 1;
  return 0;
}

void TranspositionTable::write_entry(Position* pos, int flag, int score, int depth, Move move) {
  // always replace scheme
  TTEntry *hash_entry = &table[pos->hash_key % hash_size];

  // store mate score independently from distance to root node
  if (score < - mate_score) score -= pos->ply;
  if (score > mate_score) score += pos->ply;

  hash_entry->key = pos->hash_key;
  hash_entry->depth = depth;
  hash_entry->flag = flag;
  hash_entry->score = score;
  hash_entry->best_move = move;
}

int repetition_detection(Position* pos) {
  if (!pos->ply) return 0;
  for (int index = 0; index < pos->repetition_index; index++) {
    if (pos->repetition_table[index] == pos->hash_key)
      return 1;
  }
  return 0;
}

int quiescence(Position* pos, int alpha, int beta, long& nodes) {
  // every 2047 nodes
  if((nodes & 2047 ) == 0)
    // "listen" to the GUI/user input
    communicate();

  // increment nodes count
  nodes++;

  TTEntry tte;
  Move previous_best_move = UNDEFINED_MOVE;
  int score;

  if (TT.probe(pos, tte)) {
    if (tte.flag == ExactFlag) {
      score = tte.score;

      if (score < - mate_score) score += pos->ply;
      if (score > mate_score) score -= pos->ply;
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

  int evaluation = evaluate(pos);

  // fail-hard beta cutoff
  if (evaluation >= beta) {
    return beta;
  }

  if (pos->ply > MAX_PLY - 1)
    return evaluation;

  if (evaluation > alpha) {
    alpha = evaluation;
  }

  Orderer orderer = Orderer(pos, &sd.killer_moves, &sd.history_moves, previous_best_move);
  Move current_move;

  while ((current_move = orderer.next_move()) != UNDEFINED_MOVE) {
    Position next_pos = Position(pos);

    if (!make_move(&next_pos, current_move, only_captures))  {
      continue;
    }
  
    score = -quiescence(&next_pos, -beta, -alpha, nodes);

    if(get_stop_flag()) return 0;
    
    if (score > alpha) {
      alpha = score;
      if (score >= beta) {
        return beta;
      }
    }

  }
  return alpha;
}

int negamax(Position* pos, int alpha, int beta, int depth, int null_pruning, long& nodes) {
  // every 2047 nodes
  if((nodes & 2047 ) == 0)
    // "listen" to the GUI/user input
    communicate();

  if (repetition_detection(pos))
    return draw_value;

  TTEntry tte;
  Move previous_best_move = UNDEFINED_MOVE;
  int pv_node = (beta - alpha) > 1;
  int original_alpha = alpha;
  int score;

  if (TT.probe(pos, tte)) {
    if (pos->ply && tte.depth >= depth && !pv_node) {
      if (tte.flag == ExactFlag) {
        score = tte.score;

        if (score < - mate_score) score += pos->ply;
        if (score > mate_score) score -= pos->ply;
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

  sd.pv_length[pos->ply] = pos->ply;
  int in_check = is_square_attacked(pos, (pos->side == white) ? get_lsb_index(pos->bitboards[K]) :
                                                                get_lsb_index(pos->bitboards[k]),
                                                                ~pos->side);

  // increase search depth if king has been exposed to check
  if (in_check)
    depth++;

  if (depth == 0)
    return quiescence(pos, alpha, beta, nodes);

  // static evaluation
  if (pos->ply > MAX_PLY - 1)
    return evaluate(pos);

  // increment nodes count
  nodes++;
  int legal_moves = 0;

  Orderer orderer = Orderer(pos, &sd.killer_moves, &sd.history_moves, previous_best_move);
  Move best_move = UNDEFINED_MOVE;
  int best_score = -infinity;

  int moves_searched = 0;
  Move current_move;
  while ((current_move = orderer.next_move()) != UNDEFINED_MOVE) {
    if (get_stop_flag()) return 0;
    Position next_pos = Position(pos);

    if (!make_move(&next_pos, current_move, all_moves)) {
      continue;
    }
    legal_moves++;

    // score = - negamax(next_pos, -beta, -alpha, depth-1, 0, nodes);

    // full depth search
    if (moves_searched == 0)
      // do normal alpha beta search
      score = -negamax(&next_pos, -beta, -alpha, depth - 1, 1, nodes);
    // late move reduction (LMR)
    else
    {
      // condition to consider LMR
      if(lmr_condition(current_move, moves_searched, in_check, depth)) {
        // search current move with reduced depth:
        score = -negamax(&next_pos, -alpha - 1, -alpha, depth - 2, 1, nodes);
      }
      else {
        score = alpha + 1;
      }
      
      // PVS
      if(score > alpha)
      {
        score = -negamax(&next_pos, -alpha - 1, -alpha, depth-1, 1, nodes);
    
        if((score > alpha) && (score < beta))
          score = -negamax(&next_pos, -beta, -alpha, depth-1, 1, nodes);
      }
    }

    moves_searched++;

    if (score > best_score) {

      best_score = score;
      best_move = current_move;

      if (score > alpha) {
        
        // store history moves
        if (!get_move_capture_f(current_move))
          sd.history_moves[get_move_piece(current_move)][get_move_target(current_move)] += depth;

        alpha = score;

        // write PV move
        sd.pv_table[pos->ply][pos->ply] = current_move;
              
        // loop over the next ply
        for (int next_ply = pos->ply + 1; next_ply < sd.pv_length[pos->ply + 1]; next_ply++)
          // copy move from deeper ply into a current ply's line
          sd.pv_table[pos->ply][next_ply] = sd.pv_table[pos->ply + 1][next_ply];
        
        // adjust PV length
        sd.pv_length[pos->ply] = sd.pv_length[pos->ply + 1];

        // fail-hard beta cutoff
        if (score >= beta) {

          // store killer moves
          if (!get_move_capture_f(current_move)) {
            sd.killer_moves[1][pos->ply] = sd.killer_moves[0][pos->ply];
            sd.killer_moves[0][pos->ply] = current_move;
          }
          break;
        }
      }
    }
  }

  if (legal_moves == 0) {
    if (in_check) {
      return -mate_value + pos->ply; // number of move to get to mate (fav. faster mate)
    }
    else
      return 0;
  }

  int tt_flag = (best_score >= beta) ? LowerBound : (alpha > original_alpha) ? ExactFlag : UpperBound;
  int return_score = (best_score >= beta) ? beta : (alpha > original_alpha) ? best_score  : alpha;

  TT.write_entry(pos, tt_flag, return_score, depth, best_move);
  return return_score;
}

void search_position(Position* pos, int depth) {

  pos->ply = 0;

  int score = 0;
  long nodes = 0;

  clear_search_data();

  // init alpha beta
  int alpha = -infinity, beta = infinity;

  int top_time = get_time_ms();
  reset_stop_flag();
  Move bestmove = UNDEFINED_MOVE;

  // iterative deepening
  for (int current_depth = 1; current_depth <= depth; current_depth++) {
    // if time is up
    if(get_stop_flag())
			// stop calculating and return best move so far 
			break;

    // find best move
    score = negamax(pos, alpha, beta, current_depth, 0, nodes);

    // window fail go for full window
    if ((score <= alpha) || (score >= beta)) {
      alpha = -infinity;
      beta = infinity;

      // printf("windows fail rerun at full length\n");
      score = negamax(pos, alpha, beta, current_depth, 0, nodes);
    }

    if(get_stop_flag())
			// stop calculating and return best move so far 
			break;

    bestmove = sd.pv_table[0][0];

    // aspiration window
    // alpha = score - 50;
    // beta = score + 50;
    
    if (score > -mate_value && score < -mate_score) {
      printf("info score mate %d depth %d nodes %ld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - top_time);
    }
    else if (score > mate_score && score < mate_value) {
      printf("info score mate %d depth %d nodes %ld time %d pv ", (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - top_time); 
    }  
    else
      printf("info score cp %d depth %d nodes %ld time %d pv ", score, current_depth, nodes, get_time_ms() - top_time);
    

    for (int count = 0; count < sd.pv_length[0]; count++) {
      print_move(sd.pv_table[0][count]);
      printf(" ");
    }
    printf("\n");

    if ((score > -mate_value && score < -mate_score) || (score > mate_score && score < mate_value))
      break;
  }

  printf("bestmove ");
  print_move(bestmove);
  printf("\n");

}

int lmr_condition(Move move, int moves_searched, int in_check, int depth) {
  return  (moves_searched >= full_depth_moves) &&
          (depth >= reduction_limit) &&
          (in_check == 0) && 
          (get_move_capture_f(move) == 0) &&
          (get_move_promoted(move) == 0);
}

void reset_TT() {
  TT.clear();
}
