#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_map>

#include "definition.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "utils.hpp"
#include "perft.hpp"
#include "search.hpp"
#include "uci.hpp"

void init_all() {
  init_evaluation_masks();
  init_attacks();
  init_random_keys();
}

int main() {
  // init all
  init_all();
  
  // debug mode variable
  int debug = 0;
  #if DEBUG
  debug = 1;
  #endif

  if (debug) {
    printf("debug mode\n");
    // uci_loop();

    Position pos[1];
    pos->set("4R3/1p3rk1/p4p1p/8/8/5P2/1PP3pB/2K5 b - - 1 36");
    long n=0;

    printf("Evaluation: %d\n", quiescence(pos, -50000, 5000, n));
  }
  else
    uci_loop();
  
  return 0;
}
