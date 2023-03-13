#include "perft.hpp"

#include <stdio.h>

void perft_driver(Position* pos, int depth, long& nodes) {
  // reccursion espace condition
  if (depth == 0) {
    nodes++;
    return;
  }
  else {

    MoveList move_list[1];
    generate_moves(pos, move_list);

    for (int count = 0; count < move_list->move_count; count++) {
      Position next_pos = Position(pos);

      if (!make_move(&next_pos, move_list->moves[count].move, all_moves))
        continue;

       perft_driver(&next_pos, depth - 1, nodes);
    }
  }
}

void perft_test(Position* pos, int depth) {
  printf("\n\n Performance test\n\n");

  long start = get_time_ms();
  long nodes = 0;

  MoveList move_list[1];
  generate_moves(pos, move_list);

  for (int count = 0; count < move_list->move_count; count++) {
    Position next_pos = Position(pos);

    if (!make_move(&next_pos, move_list->moves[count].move, all_moves))
      continue;

    long cummulative_nodes = nodes;

    perft_driver(&next_pos, depth - 1, nodes);

    long old_nodes = nodes - cummulative_nodes;
    printf("    move: %s%s%c  nodes : %ld\n", square_to_coordinates[get_move_source(move_list->moves[count].move)],
                                              square_to_coordinates[get_move_target(move_list->moves[count].move)],
                                              (get_move_promoted(move_list->moves[count].move)) ? ascii_pieces[get_move_promoted(move_list->moves[count].move)] : ' ',
                                              old_nodes);
  }

  printf("\n    Depth: %d", depth);
  printf("\n    Nodes: %ld", nodes);
  printf("\n    Time: %lldms\n\n", get_time_ms() - start);

}