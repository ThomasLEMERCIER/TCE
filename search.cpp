#include "search.hpp"

#include <stdio.h>

SearchData sd;
TranspositionTable TT;
int follow_pv, score_pv;


void TranspositionTable::clear() {
  memset(this, 0, sizeof(TranspositionTable));
}

void clear_search_data() {
  memset(&sd, 0, sizeof(SearchData));
}

int TranspositionTable::probe(Position* pos, int alpha, int beta, int depth) {
  TTEntry *hash_entry = &table[pos->hash_key % hash_size];

  if (hash_entry->key == pos->hash_key) {
    if (hash_entry->depth >= depth) {
      if (hash_entry->flag == hash_flag_exact) {
        int score = hash_entry->score;

        if (score < - mate_score) score += pos->ply;
        if (score > mate_score) score -= pos->ply;
        return score;
      }
      if ((hash_entry->flag == hash_flag_alpha) &&  hash_entry->score <= alpha) {
        return alpha;
      }
      if ((hash_entry->flag == hash_flag_beta) &&  hash_entry->score >= beta) {
        return beta;
      }
    }
  }
  return no_hash_entry;
}

void TranspositionTable::write_entry(Position* pos, int flag, int score, int depth) {
  // always replace scheme
  TTEntry *hash_entry = &table[pos->hash_key % hash_size];

  // store mate score independently from distance to root node
  if (score < - mate_score) score -= pos->ply;
  if (score > mate_score) score += pos->ply;

  hash_entry->key = pos->hash_key;
  hash_entry->depth = depth;
  hash_entry->flag = flag;
  hash_entry->score = score;
}

int score_move(Position* pos, Move move) {
  // PV move scoring is allowed
  if (score_pv) {
    // dealing with the PV move
    if (sd.pv_table[0][pos->ply] == move) {
      // disable score pv flag
      score_pv = 0;

      // give PV move the highest score to search first
      return 20000;
    }
  }

  // score capture move
  if (get_move_capture_f(move)) {
    Piece target_piece = Piece::P;

    Piece start_piece = (pos->side == Color::white) ? Piece::p : Piece::P;
    Piece end_piece = (pos->side == Color::white) ? Piece::k : Piece::K;

    for (Piece bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
      if (get_bit(pos->bitboards[bb_piece], get_move_target(move))) {
        target_piece = bb_piece;
        break;
      }
    }

    return mvv_lva[get_move_piece(move)][target_piece] + 10000;
  }

  // score quiet move
  else {
    // score 1st killer move
    if (sd.killer_moves[0][pos->ply] == move)
      return 9000;

    // score 2nd killer move
    else if (sd.killer_moves[1][pos->ply] == move)
      return 8000;

    // score history move
    else
      return sd.history_moves[get_move_piece(move)][get_move_target(move)];
  }

  return 0;
}

void enable_pv_scoring(Position* pos, MoveList *move_list) {
  // disable following pv
  follow_pv = 0;

  for (int count = 0; count < move_list->move_count; count++) {
    if (sd.pv_table[0][pos->ply] == move_list->moves[count]) {
      score_pv = 1;
      follow_pv = 1;
    }
  }
}

void sort_moves(Position* pos, MoveList *move_list) {
  int move_scores[move_list->move_count];

  for (int count = 0; count < move_list->move_count; count++) {
    move_scores[count] = score_move(pos, move_list->moves[count]);
  }

  for (int current_move = 0; current_move < move_list->move_count; current_move++) {
      for (int next_move = current_move + 1; next_move < move_list->move_count; next_move++) {
          if (move_scores[current_move] < move_scores[next_move]) {
              int temp_score = move_scores[current_move];
              move_scores[current_move] = move_scores[next_move];
              move_scores[next_move] = temp_score;

              Move temp_move = move_list->moves[current_move];
              move_list->moves[current_move] = move_list->moves[next_move];
              move_list->moves[next_move] = temp_move;
          }
      }
  }
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

  MoveList move_list[1];
  generate_moves(move_list, pos);
  sort_moves(pos, move_list);

  for (int count = 0; count < move_list->move_count; count++) {
    Position next_pos[1];
    copy_position(next_pos, pos);

    if (!make_move(next_pos, move_list->moves[count], only_captures))  {
      continue;
    }
  
    int score = -quiescence(next_pos, -beta, -alpha, nodes);

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

  int pv_node = (beta - alpha) > 1;
  int score = TT.probe(pos, alpha, beta, depth);

  if (pos->ply && (score != no_hash_entry) && !pv_node) {
    return score;
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

  // null move pruning
  // if (null_pruning && (!pv_node && depth >= 3 && !in_check && pos->ply)) {

  //   Position next_pos[1];
  //   copy_position(next_pos, pos);
  //   make_null_move(next_pos);

  //   // reduction factor
  //   int r = 2;

  //   // disable null pruning for next node
  //   score = -negamax(next_pos, -beta, - beta + 1, depth - 1 - r, 0, nodes);

  //   // // return 0 if time is up
  //   // if(stopped == 1) return 0;

  //   if (score >= beta) {
  //     return beta;
  //   }
  // }

  MoveList move_list[1];
  generate_moves(move_list, pos);

  if (follow_pv)
    enable_pv_scoring(pos, move_list);

  sort_moves(pos, move_list);

  int moves_searched = 0;

  for (int count = 0; count < move_list->move_count; count++) {
    Position next_pos[1];
    copy_position(next_pos, pos);

    if (!make_move(next_pos, move_list->moves[count], all_moves)) {
      continue;
    }
    legal_moves++;

    // score = - negamax(next_pos, -beta, -alpha, depth-1, 0, nodes);

    // full depth search
    if (moves_searched == 0)
      // do normal alpha beta search
      score = -negamax(next_pos, -beta, -alpha, depth - 1, 1, nodes);
    // late move reduction (LMR)
    else
    {
      // condition to consider LMR
      if(lmr_condition(move_list->moves[count], moves_searched, in_check, depth)) {
        // search current move with reduced depth:
        score = -negamax(next_pos, -alpha - 1, -alpha, depth - 2, 1, nodes);
      }
      else {
        score = alpha + 1;
      }
      
      // principle variation search PVS
      if(score > alpha)
      {
        score = -negamax(next_pos, -alpha - 1, -alpha, depth-1, 1, nodes);
    
        if((score > alpha) && (score < beta))
          score = -negamax(next_pos, -beta, -alpha, depth-1, 1, nodes);
      }
    }

    moves_searched++;

    if (get_stop_flag()) return 0;

    // found a better move
    if (score > alpha) {
      TT.write_entry(pos, hash_flag_exact, score, depth);

      // store history moves
      if (!get_move_capture_f(move_list->moves[count]))
        sd.history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;

      // PV node
      alpha = score;

      // write PV move
      sd.pv_table[pos->ply][pos->ply] = move_list->moves[count];
            
      // loop over the next ply
      for (int next_ply = pos->ply + 1; next_ply < sd.pv_length[pos->ply + 1]; next_ply++)
        // copy move from deeper ply into a current ply's line
        sd.pv_table[pos->ply][next_ply] = sd.pv_table[pos->ply + 1][next_ply];
      
      // adjust PV length
      sd.pv_length[pos->ply] = sd.pv_length[pos->ply + 1];

      // fail-hard beta cutoff
      if (score >= beta) {
        TT.write_entry(pos, hash_flag_beta, score, depth);

        // store killer moves
        if (!get_move_capture_f(move_list->moves[count])) {
          sd.killer_moves[1][pos->ply] = sd.killer_moves[0][pos->ply];
          sd.killer_moves[0][pos->ply] = move_list->moves[count];
        }
        return beta;
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
  TT.write_entry(pos, hash_flag_alpha, score, depth);
  return alpha;
}

void search_position(Position* pos, int depth) {

  pos->ply = 0;

  int score = 0;
  long nodes = 0;

  // reset follow pv flags
  follow_pv = 0;
  score_pv = 0;

  clear_search_data();

  // init alpha beta
  int alpha = -infinity, beta = infinity;

  int starttime = get_time_ms();
  reset_stop_flag();
  Move bestmove;

  // iterative deepening
  for (int current_depth = 1; current_depth <= depth; current_depth++) {
    // if time is up
    if(get_stop_flag())
			// stop calculating and return best move so far 
			break;

    printf("Current depth: %d\n", current_depth);

    // enable follow pv flag
    follow_pv = 1;

    // find best move
    score = negamax(pos, alpha, beta, current_depth, 0, nodes);

    // window fail go for full window
    if ((score <= alpha) || (score >= beta)) {
      alpha = -infinity;
      beta = infinity;

      printf("windows fail rerun at full length\n");
      score = negamax(pos, alpha, beta, current_depth, 0, nodes);
    }

    bestmove = sd.pv_table[0][0];

    if(get_stop_flag())
			// stop calculating and return best move so far 
			break;

    // aspiration window
    // alpha = score - 50;
    // beta = score + 50;
    
    if (score > -mate_value && score < -mate_score) {
      printf("info score mate %d depth %d nodes %ld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - starttime);
    }
    else if (score > mate_score && score < mate_value) {
      printf("info score mate %d depth %d nodes %ld time %d pv ", (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - starttime); 
    }  
    else
      printf("info score cp %d depth %d nodes %ld time %d pv ", score, current_depth, nodes, get_time_ms() - starttime);
    

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
