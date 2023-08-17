#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include "position.hpp"
#include "definition.hpp"
#include "bitboard.hpp"
#include "definition.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "evaluate.hpp"
#include "timeman.hpp"
#include "orderer.hpp"
#include "ttable.hpp"

#include <atomic>

extern std::atomic<bool> search_stopped;

struct ThreadData {
  int thread_id;
  int depth;
  Position pos;

  NodeCounter nodes = 0;

  KillerMoves killer_moves{};
  HistoryMoves history_moves{};
  int pv_length[MAX_PLY_SEARCH]{};
  Move pv_table[MAX_PLY_SEARCH][MAX_PLY_SEARCH]{};

  SearchLimits limits;
};

void start_search(const Position& pos, int depth, const SearchLimits& limits);
void stop_search();

int quiescence(Position* pos, int alpha, int beta, ThreadData& td);
int negamax(Position* pos, int alpha, int beta, int depth, int null_pruning, ThreadData& td);

void search_position(ThreadData& td);

constexpr int full_depth_moves = 4;
constexpr int lmr_reduction = 3;
constexpr int null_move_reduction = 3;

constexpr int check_every_nodes = 2046;

#endif
