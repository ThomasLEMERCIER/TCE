#pragma once

#include <sstream>

#include "definition.hpp"
#include "position.hpp"

Move parse_move(Position* pos, std::string move_string);
void parse_position(Position& pos, std::istringstream& ss, bool& position_set);
void parse_go(Position& pos, std::istringstream& ss, const bool& position_set);
void uci_loop(int argc, char* argv[]);
