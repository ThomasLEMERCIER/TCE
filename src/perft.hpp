#pragma once

#include "position.hpp"
#include "movegen.hpp"
#include "timeman.hpp"

void perft_driver(Position* pos, int depth, long& nodes);
void perft_test(Position* pos, int depth);
