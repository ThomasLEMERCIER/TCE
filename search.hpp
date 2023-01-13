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

#define infinity 50000
#define mate_value 49000
#define mate_score 48000
#define draw_value 0

constexpr int MAX_PLY = 64;

extern int killer_moves [2][MAX_PLY];
extern int history_moves[12][64];
extern int pv_length[MAX_PLY];
extern int pv_table[MAX_PLY][MAX_PLY];
extern int follow_pv, score_pv;

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


#define hash_size 800000
#define no_hash_entry 100000

#define hash_flag_exact  0
#define hash_flag_alpha  1
#define hash_flag_beta   2

struct TTEntry {
  U64 key;    // hash key of position
  int depth;  // depth to get value
  int flag;   // flag for the type of node (fail-high/fail-low/PV)
  int score;  // score (beta/alpha/PV)
};

class TranspositionTable {
public:
  TTEntry table[hash_size];


  int probe(Position* pos, int alpha, int beta, int depth);
  void write_entry(Position* pos, int flag, int score, int depth);
  void clear();
};
extern TranspositionTable TT;


int score_move(Position* pos, Move move);
void enable_pv_scoring(Position* pos, MoveList *move_list);
void sort_moves(Position* pos, MoveList *move_list);
int quiescence(Position* pos, int alpha, int beta, long& nodes);
int negamax(Position* pos, int alpha, int beta, int depth, int null_pruning, long& nodes);


int lmr_condition(Move move, int moves_searched, int in_check, int depth);

void search_position(Position* pos, int depth);
void reset_TT();

constexpr int full_depth_moves = 4;
constexpr int reduction_limit = 3;

/*                       
    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/
// MVV LVA [attacker][victim]
const int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

#endif
