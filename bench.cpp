#include "bench.hpp"

#include <stdio.h>

void bench() {

  SearchLimits limits = {};
  NodeCounter nodes = 0;
  Position pos;

  TimePoint top_time = get_time_ms();
  for (int i_fen = 0; i_fen < BENCH_FEN_NB; ++i_fen) {
    printf("Current position fen: %s\n", bench_fens[i_fen]);

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
  printf("\n===========================\nTotal time (ms) : %lld, \nNodes searched  : %llu, \nNodes/second    : %f\n", elapsed, nodes, nps);
  printf("%llu nodes %i nps", nodes, static_cast<int>(nps));
}