#include "bench.hpp"

#include <stdio.h>

void bench() {

  Position pos;
  unsigned long long nodes = 0;
  int top_time = get_time_ms();

  for (int i_fen = 0; i_fen < BENCH_FEN_NB; ++i_fen) {
    printf("Current position fen: %s\n", bench_fens[i_fen]);
    pos.set(bench_fens[i_fen]);
    
    search_position(&pos, 8, nodes);
  }
  int elapsed = get_time_ms() - top_time;
  printf("\n===========================\nTotal time (ms) : %d, \nNodes searched  : %llu, \nNodes/second    : %f\n", elapsed, nodes, 1000 * (double)((double)nodes / (double)elapsed));
  printf("%llu nodes %llu nps", nodes, 1000 * nodes / elapsed);
}