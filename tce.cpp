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
    Position pos[1];

    pos->set("4r1k1/ppp3pp/n4p2/3Ppq2/4R2P/5N2/PPP2RP1/6K1 w - - 0 26");
    printf("Evaluation: %d\n", evaluate(pos));
  }
  else
    uci_loop();
  
  return 0;
}
