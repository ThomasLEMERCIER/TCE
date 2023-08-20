#pragma once

#include "definition.hpp"
#include "position.hpp"
#include "move.hpp"
#include "timeman.hpp"

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

Score quiescence(Position* pos, Score alpha, Score beta, ThreadData& td);
Score negamax(Position* pos, Score alpha, Score beta, int depth, bool null_pruning, ThreadData& td);
Score aspiration_window(Position* pos, Score previous_score, int depth, ThreadData& td);

void search_position(ThreadData& td);
constexpr int check_every_nodes = 2046;

// =========== search hyperparameters ===========
// === LMR ===
constexpr int full_depth_moves = 4;
constexpr int lmr_reduction = 3;

// === Null move pruning ===
constexpr int null_move_reduction = 3;

// === Aspiration window ===
constexpr int aspiration_window_depth = 5;
constexpr Score aspiration_window_start = 150;
constexpr Score aspiration_window_decrement = 0;
constexpr Score aspiration_window_end = 10;
constexpr Score aspiration_window_max = 350;
// ===============================================


