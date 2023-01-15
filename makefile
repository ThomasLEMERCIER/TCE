### Source and object files
SRCS = bitboard.cpp definition.cpp evaluate.cpp move.cpp movegen.cpp perft.cpp position.cpp rng.cpp search.cpp tce.cpp uci.cpp utils.cpp
# OBJS = $(notdir $(SRCS:.cpp=.o))

$(info Source file: $(SRCS))
$(info Object file: $(OBJS))

MANDATORY_FLAG = -Wall -Wcast-qual -fno-exceptions -std=c++17 -pedantic -Wextra -Wshadow -Wmissing-declarations
OPTI_FLAG = -O3
DEBUG_FLAG = -DDEBUG

release:
	g++ $(MANDATORY_FLAG) $(OPTI_FLAG) $(SRCS) -o tce.exe
debug:
	g++ $(MANDATORY_FLAG) $(OPTI_FLAG) $(DEBUG_FLAG) $(SRCS) -o tce_d.exe
