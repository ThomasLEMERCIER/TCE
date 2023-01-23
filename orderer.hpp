#ifndef ORDERER_H_INCLUDED
#define ORDERER_H_INCLUDED

#include "movegen.hpp"
#include "move.hpp"
#include "search.hpp"
#include "definition.hpp"

#define CAP_SCORE_SORT        10000
#define BEST_SCORE_SORT       100000
#define FIRST_KILLER_SORT     5000
#define SECOND_KILLER_SORT    3000

class Orderer {
public:
  Orderer(Position* pos, KillerMoves* killer_moves, HistoryMoves* history_moves, Move previous_best_move);
  void score_moves();
  Move next_move();

  int current_idx;
  MoveList move_list;

  Position* pos;

  HistoryMoves* history_moves;
  KillerMoves* killer_moves;  
  Move previous_best_move;
};

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
constexpr int mvv_lva[12][12] = {
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
