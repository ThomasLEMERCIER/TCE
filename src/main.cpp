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
#include "timeman.hpp"
#include "perft.hpp"
#include "search.hpp"
#include "uci.hpp"

void init_all();

void init_all() {
  init_evaluation_masks();
  init_attacks();
  init_random_keys();
}

int main(int argc, char **argv) {
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

    pos->set(start_position);
    perft_test(pos, 6);

    pos->set(tricky_position);
    perft_test(pos, 5);

    pos->set(killer_position);
    perft_test(pos, 5);

    pos->set(perft_3);
    perft_test(pos, 6);

    pos->set(perft_4);
    perft_test(pos, 5);

    pos->set(perft_5);
    perft_test(pos, 5);

  }
  else
    uci_loop(argc, argv);
  
  return 0;
}
