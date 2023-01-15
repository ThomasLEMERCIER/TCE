#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include "utils.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "search.hpp"
// exit from engine flag
extern int quit;

// UCI "movestogo" command moves counter
extern int movestogo;

// UCI "movetime" command time counter
extern int movetime;

// UCI "time" command holder (ms)
extern int time_holder;

// UCI "inc" command's time increment holder
extern int inc;

// UCI "starttime" command time holder
extern int starttime;

// UCI "stoptime" command time holder
extern int stoptime;

// variable to flag time control availability
extern int timeset;

// variable to flag when the time is up
extern int stopped;

int input_waiting();
void read_input();
void communicate();
int parse_move(Position* pos, char *move_string);
void parse_position(Position* pos, char *command);
void parse_go(Position* pos, char *command);
void uci_loop();
int get_stop_flag();
void reset_stop_flag();

#endif
