#include "uci.hpp"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <windows.h>

int quit = 0;
int movestogo = 30;
int movetime = -1;
int time_holder = -1;
int inc = 0;
int starttime = 0;
int stoptime = 0;
int timeset = 0;
int stopped = 0;

int input_waiting() {
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init)
  {
      init = 1;
      inh = GetStdHandle(STD_INPUT_HANDLE);
      pipe = !GetConsoleMode(inh, &dw);
      if (!pipe)
      {
          SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
          FlushConsoleInputBuffer(inh);
      }
  }
  
  if (pipe)
  {
      if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
      return dw;
  }
  
  else
  {
      GetNumberOfConsoleInputEvents(inh, &dw);
      return dw <= 1 ? 0 : dw;
  }
}

// read GUI/user input
void read_input() {
  // bytes to read holder
  int bytes;
  
  // GUI/user input
  char input[256] = "", *endc;

  // "listen" to STDIN
  if (input_waiting())
  {
    // tell engine to stop calculating
    stopped = 1;
    
    // loop to read bytes from STDIN
    do
    {
      // read bytes from STDIN
      bytes=read(fileno(stdin), input, 256);
    }
    
    // until bytes available
    while (bytes < 0);
    
    // searches for the first occurrence of '\n'
    endc = strchr(input,'\n');
    
    // if found new line set value at pointer to 0
    if (endc) *endc=0;
    
    // if input is available
    if (strlen(input) > 0)
    {
      // match UCI "quit" command
      if (!strncmp(input, "quit", 4))
      {
        // tell engine to terminate exacution    
        quit = 1;
      }

      // // match UCI "stop" command
      else if (!strncmp(input, "stop", 4))    {
        // tell engine to terminate exacution
        quit = 1;
      }
    }   
  }
}

// a bridge function to interact between search and GUI input
void communicate() {
	// if time is up break here
    if(timeset == 1 && get_time_ms() > stoptime) {

    printf("Stopping\n");
		// tell engine to stop calculating
		stopped = 1;
	}
	
    // read GUI input
	read_input();
}

Move parse_move(Position* pos, char *move_string) {
  // generate moves
  MoveList move_list[1];
  generate_moves(pos, move_list);

  // parse squares
  int source_square = (move_string[0] - 'a') + 8 * (8 - (move_string[1] - '0'));
  int target_square = (move_string[2] - 'a') + 8 * (8 - (move_string[3] - '0'));

  for (int count = 0; count < move_list->move_count; count++) {
    Move move = move_list->moves[count].move;

    if (source_square == get_move_source(move) && target_square == get_move_target(move)) {

      int promoted_piece = get_move_promoted(move);

      if (promoted_piece) {
      if ((promoted_piece ==  Q || promoted_piece == q) && move_string[4] == 'q')
        return move;

      if ((promoted_piece ==  N || promoted_piece == n) && move_string[4] == 'n')
        return move;
      
      if ((promoted_piece ==  R || promoted_piece == r) && move_string[4] == 'r')
        return move;

      if ((promoted_piece ==  B || promoted_piece == b) && move_string[4] == 'b')
        return move;

      continue;
      }

      return move;
    }
  }

  return 0;  
}

void parse_position(Position* pos, char *command) {
  // shift pointer at next token
  command += 9;

  // init pointer to current character in the command string
  char *current_char = command;

  // parse UCI "startpos" command
  if (strncmp(command, "startpos", 8) == 0) {
    // init chess board with start position
    pos->set(start_position);
  }
  // parse UCI "fen" command
  else {
    // make sure "fen" command is available in the command string
    current_char = strstr(command, "fen");

    if (current_char == NULL)
      pos->set(start_position);
    else {
      current_char += 4;

      pos->set(current_char);
    }
  }

  current_char = strstr(command, "moves");

  // moves available
  if (current_char != NULL) {
    // shift pointer to the next token
    current_char += 6;

    // loop over moves
    while (*current_char) {

      Move move = parse_move(pos, current_char);

      if (move == 0)
        break;

      make_move(pos, move, all_moves);

      // move pointer to end of current move
      while (*current_char && *current_char != ' ') current_char++;

      // move pointer to the next move
      current_char++;
    }
  }

  // printf("position set\n");
}

void parse_go(Position* pos, char *command)
{
  // init parameters
  int depth = -1;

  // init argument
  char *argument = NULL;

  // infinite search
  if ((argument = strstr(command,"infinite"))) {}

  // match UCI "binc" command
  if ((argument = strstr(command,"binc")) && pos->side == black)
    // parse black time increment
    inc = atoi(argument + 5);

  // match UCI "winc" command
  if ((argument = strstr(command,"winc")) && pos->side == white)
    // parse white time increment
    inc = atoi(argument + 5);

  // match UCI "wtime" command
  if ((argument = strstr(command,"wtime")) && pos->side == white)
    // parse white time limit
    time_holder = atoi(argument + 6);

  // match UCI "btime" command
  if ((argument = strstr(command,"btime")) && pos->side == black)
    // parse black time limit
    time_holder = atoi(argument + 6);

  // match UCI "movestogo" command
  if ((argument = strstr(command,"movestogo")))
    // parse number of moves to go
    movestogo = atoi(argument + 10);

  // match UCI "movetime" command
  if ((argument = strstr(command,"movetime")))
    // parse amount of time allowed to spend to make a move
    movetime = atoi(argument + 9);

  // match UCI "depth" command
  if ((argument = strstr(command,"depth")))
    // parse search depth
    depth = atoi(argument + 6);

  // if move time is not available
  if(movetime != -1)
  {
    // set time equal to move time
    time_holder = movetime;

    // set moves to go to 1
    movestogo = 1;
  }

  // init start time
  starttime = get_time_ms();

  // init search depth
  depth = depth;

  // if time control is available
  if(time_holder != -1)
  {
    // flag we're playing with time control
    timeset = 1;

    // set up timing
    time_holder /= movestogo;
    time_holder -= 50;
    stoptime = starttime + time_holder + inc;
  }

  // if depth is not available
  if(depth == -1)
    // set depth to 64 plies (takes ages to complete...)
    depth = 64;

  // print debug info
  printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
  time_holder, starttime, stoptime, depth, timeset);

  // search position
  search_position(pos, depth);
}

int get_stop_flag() {
  return stopped;
}

void reset_stop_flag() {
  stopped = 0;
}

void uci_loop() {
  // reset STDIN & STDOUT buffers
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  // define user / GUI input buffer
  char input[2000];

  Position pos[1];

  // main loop
  while (1) {
    // reset user / GUI input
    memset(input, 0, sizeof(input));

    // make sure output reacher the GUI
    fflush(stdout);

    // get user / GUI input
    if (!fgets(input, 2000, stdin))
      continue;

    if (input[0] == '\n')
      continue;

    // parse "isready" command
    if (strncmp(input, "isready", 7) == 0)
      printf("readyok\n");

    // parse "ucinewgame" command
    else if (strncmp(input, "ucinewgame", 10) == 0) {
      reset_TT();
    }

    // parse UCI "position" command
    else if (strncmp(input, "position", 8) == 0) {
      parse_position(pos, input);
    }

    // parse UCI "go" command
    else if (strncmp(input, "go", 2) == 0)
      parse_go(pos, input);

    // parse UCI "quit" command
    else if (strncmp(input, "quit", 4) == 0)
      break;

    // parse UCI "uci" command
    else if (strncmp(input, "uci", 3) == 0)
    {
      // print engine info
      printf("id name TCE\n");
      printf("id name TCE\n");
      printf("uciok\n");
    }
  }
}