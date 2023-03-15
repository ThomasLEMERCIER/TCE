#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include <sstream>

#include "timeman.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "search.hpp"
#include "bench.hpp"
#include "definition.hpp"
#include "ttable.hpp"

int parse_move(Position* pos, std::string move_string);
void parse_position(Position& pos, std::istringstream& ss, bool& position_set);
void parse_go(Position& pos, std::istringstream& ss, const bool& position_set);
void uci_loop(int argc, char* argv[]);

#endif
