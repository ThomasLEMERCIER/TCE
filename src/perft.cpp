#include "perft.hpp"

#include "movegen.hpp"
#include "timeman.hpp"

#include <iostream>

void perft_driver(Position* pos, int depth, NodeCounter& nodes) {
  // recursion escape condition
  if (depth == 0) {
    nodes++;
    return;
  }
  else {

    MoveList move_list[1];
    generate_moves(pos, move_list);

    for (int count = 0; count < move_list->move_count; count++) {
      Position next_pos = Position(pos);

      if (!make_move(&next_pos, move_list->moves[count].move, Move_Type::ALL_MOVES))
        continue;

       perft_driver(&next_pos, depth - 1, nodes);
    }
  }
}

void perft_test(Position* pos, int depth) {
  std::cout << "\n\n Performance test\n\n";

  TimePoint start = get_time_ms();
  NodeCounter nodes = 0;

  MoveList move_list[1];
  generate_moves(pos, move_list);

  for (int count = 0; count < move_list->move_count; count++) {
    Position next_pos = Position(pos);

    if (!make_move(&next_pos, move_list->moves[count].move, Move_Type::ALL_MOVES))
      continue;

    NodeCounter cummulative_nodes = nodes;

    perft_driver(&next_pos, depth - 1, nodes);

    NodeCounter old_nodes = nodes - cummulative_nodes;
    std::cout << "    move: " << square_to_coordinates[get_move_source(move_list->moves[count].move)]
      << square_to_coordinates[get_move_target(move_list->moves[count].move)]
      << ((get_move_promoted(move_list->moves[count].move)) ? ascii_pieces[get_move_promoted(move_list->moves[count].move)] : ' ')
      << "  nodes : " << old_nodes << std::endl;
  }

  std::cout << "\n    Depth: " << depth << std::endl;
  std::cout << "    Nodes: " << nodes << std::endl;
  std::cout << "    Time: " << get_time_ms() - start << "ms\n" << std::endl;
}