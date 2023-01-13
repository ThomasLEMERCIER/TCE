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

    Position pos[1];
    pos->set("1rbk3r/3n4/1p1N1p1n/3Np2p/p1B3p1/4P3/PP1B2PP/2KR3R b - - 4 4");
    print_board(pos);
    printf("Evaluation: %d\n", evaluate(pos));

    // uci_loop();
  }
  else
    uci_loop();
  
  return 0;
}
