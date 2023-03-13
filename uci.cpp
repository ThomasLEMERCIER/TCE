#include "uci.hpp"

#include <string>
#include <iostream>
#include <cstdint>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <windows.h>

Move parse_move(Position* pos, std::string move_string) {

  MoveList move_list;
  generate_moves(pos, &move_list);
  Move move;

  // parse squares
  Square source_square = get_square(Rank(8 - (move_string[1] - '0')), File(move_string[0] - 'a'));
  Square target_square = get_square(Rank(8 - (move_string[3] - '0')), File(move_string[2] - 'a'));

  for (int count = 0; count < move_list.move_count; count++) {
    move = move_list.moves[count].move;

    if (source_square == get_move_source(move) && target_square == get_move_target(move)) {

      int promoted_piece = get_move_promoted(move);

      if (promoted_piece) {
        if ((promoted_piece ==  Piece::WQ || promoted_piece == Piece::BQ) && move_string[4] == 'q') return move;
        if ((promoted_piece ==  Piece::WN || promoted_piece == Piece::BN) && move_string[4] == 'n') return move;
        if ((promoted_piece ==  Piece::WR || promoted_piece == Piece::BR) && move_string[4] == 'r') return move;
        if ((promoted_piece ==  Piece::WB || promoted_piece == Piece::BB) && move_string[4] == 'b') return move;

      continue;
      }

      return move;
    }
  }

  return UNDEFINED_MOVE;  
}

void parse_position(Position& pos, std::istringstream& ss, bool& position_set) {

  std::string token, fen;
  ss >> token;

  if (token == "startpos") {
    fen = start_position;
    ss >> token;
  }
  else if (token == "fen") {
    while (ss >> token && token != "moves")
      fen += token + " ";
  }

  pos.set(fen);

  if (token == "moves") {
    Move move;
    while (ss >> token && (move = parse_move(&pos, token)) != UNDEFINED_MOVE) {
      make_move(&pos, move, all_moves);
    }
  }
  position_set = true;
}

void parse_go(Position& pos, std::istringstream& ss, const bool& position_set) {

  if (!position_set) {
    std::cout << "info string error: position not set" << std::endl;
    return;
  }
  
  TimePoint start = get_time_ms();

  std::string token;
  SearchLimits limits = {};
  int depth;
  
  while (ss >> token) {
    if (token == "wtime") ss >> limits.times[Color::WHITE];
    else if (token == "btime") ss >> limits.times[Color::BLACK];
    else if (token == "winc") ss >> limits.incs[Color::WHITE];
    else if (token == "binc") ss >> limits.incs[Color::BLACK];
    else if (token == "movestogo") ss >> limits.movestogo;
    else if (token == "depth") ss >> limits.depth;
    else if (token == "nodes") ss >> limits.nodes;
    else if (token == "movetime") ss >> limits.movetime;
    else if (token == "infinite") limits.infinite = true;
  }

  if (limits.use_time_management()) {
    tm.init(limits, pos.side, start);
    depth = MAX_PLY_SEARCH;
  }
  else {
    depth = limits.depth ? limits.depth : MAX_PLY_SEARCH;
  }

  start_search(pos, depth, limits);
}

void uci_loop(int argc, char* argv[]) {

  if (argc > 1 && (strncmp(argv[1], "bench", 5) == 0)) {
    bench(); return;
  }

  Position pos;
  bool position_set = false;
  std::string cmd;

  // main loop
  while (1) {

    if (!std::getline(std::cin, cmd)) continue;

    std::istringstream ss(cmd);
    std::string token;
    ss >> std::skipws >> token;

    if (token == "quit")            break;
    else if (token == "stop")       stop_search();
    else if (token == "uci")        std::cout << "id name TCE\nid author Thomas Lemercier\noption \nuciok\n";
    else if (token == "isready")    std::cout << "readyok\n";
    else if (token == "ucinewgame") TT.clear();
    else if (token == "position")   parse_position(pos, ss, position_set);
    else if (token == "go")         parse_go(pos, ss, position_set);
    else if (token == "bench")      bench();
  }

  stop_search();
}