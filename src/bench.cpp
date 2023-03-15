#include "bench.hpp"

#include <iostream>

void bench() {

  SearchLimits limits = {};
  NodeCounter nodes = 0;
  Position pos;

  TimePoint top_time = get_time_ms();
  for (int i_fen = 0; i_fen < BENCH_FEN_NB; ++i_fen) {
    std::cout << "Current position fen: " << bench_fens[i_fen] << std::endl;

    pos.set(bench_fens[i_fen]);

    ThreadData td;

    td.thread_id = 0;
    td.depth = 8;
    td.pos = pos;

    td.limits = limits;

    search_position(td);
    nodes += td.nodes;
  }

  TimePoint elapsed = get_time_ms() - top_time;
  double nps = 1000.0 * static_cast<double>(nodes) / static_cast<double>(elapsed);
  std::cout << "===========================\nTotal time (ms) : " << elapsed << ", \nNodes searched  : " << nodes << ", \nNodes/second    : " << nps << '\n';
  std::cout << nodes << " nodes " << static_cast<int>(nps) << " nps" << std::endl;
}