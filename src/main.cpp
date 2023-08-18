#include "definition.hpp"
#include "position.hpp"
#include "evaluate.hpp"
#include "perft.hpp"
#include "uci.hpp"

#include <iostream>

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
    std::cout << "Debug mode" << std::endl;

    init_magic_numbers();

    // perft tests
    Position pos[1];
    pos->set(start_position);
    print_board(pos);

    print_bitboard(pos->bitboards[Piece::WP]);
    print_bitboard(pos->bitboards[Piece::BP]);
    perft_test(pos, 6);

    pos->set(tricky_position);
    print_board(pos);
    perft_test(pos, 5);

    pos->set(killer_position);
    print_board(pos);
    perft_test(pos, 5);

    pos->set(perft_3);
    print_board(pos);
    perft_test(pos, 6);

    pos->set(perft_4);
    print_board(pos);
    perft_test(pos, 5);

    pos->set(perft_5);
    print_board(pos);
    perft_test(pos, 5);
  }
  else
    uci_loop(argc, argv);
  
  return 0;
}
