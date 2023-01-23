#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include "position.hpp"
#include "definition.hpp"
#include "bitboard.hpp"
#include "definition.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "evaluate.hpp"
#include "utils.hpp"
#include "uci.hpp"
#include "orderer.hpp"

#define infinity 50000
#define mate_value 49000
#define mate_score 48000
#define draw_value 0

/*    ================================
            Triangular PV table
      --------------------------------
        PV line: e2e4 e7e5 g1f3 b8c6
      ================================
           0    1    2    3    4    5
      
      0    m1   m2   m3   m4   m5   m6
      
      1    0    m2   m3   m4   m5   m6 
      
      2    0    0    m3   m4   m5   m6
      
      3    0    0    0    m4   m5   m6
       
      4    0    0    0    0    m5   m6
      
      5    0    0    0    0    0    m6
*/

struct SearchData {
  KillerMoves killer_moves;
  HistoryMoves history_moves;
  int pv_length[MAX_PLY];
  Move pv_table[MAX_PLY][MAX_PLY];
};

#define hash_size 800000
#define no_hash_entry 100000

#define hash_flag_exact  0
#define hash_flag_alpha  1
#define hash_flag_beta   2

struct TTEntry {
  U64 key;          // hash key of position
  int depth;        // depth to get value
  int flag;         // flag for the type of node (fail-high/fail-low/PV)
  int score;        // score (beta/alpha/PV)
  Move best_move;   // best move found
};

class TranspositionTable {
public:
  TTEntry table[hash_size];


  int probe(Position* pos, int alpha, int beta, int depth, Move& move);
  void write_entry(Position* pos, int flag, int score, int depth, Move move);
  void clear();
};

extern TranspositionTable TT;
extern SearchData sd;

int quiescence(Position* pos, int alpha, int beta, long& nodes);
int negamax(Position* pos, int alpha, int beta, int depth, int null_pruning, long& nodes);


int lmr_condition(Move move, int moves_searched, int in_check, int depth);

void search_position(Position* pos, int depth);
void reset_TT();

constexpr int full_depth_moves = 4;
constexpr int reduction_limit = 3;

#endif
