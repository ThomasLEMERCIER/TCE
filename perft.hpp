#ifndef PERFT_H_INCLUDED
#define PERFT_H_INCLUDED

#include "position.hpp"
#include "movegen.hpp"
#include "utils.hpp"

void perft_driver(Position* pos, int depth, long& nodes);
void perft_test(Position* pos, int depth);

#endif