#pragma once

#include "position.hpp"

void perft_driver(Position* pos, int depth, NodeCounter& nodes);
void perft_test(Position* pos, int depth);
