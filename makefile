### Source files
SRCS = bench.cpp bitboard.cpp definition.cpp evaluate.cpp move.cpp movegen.cpp orderer.cpp perft.cpp position.cpp rng.cpp search.cpp tce.cpp uci.cpp utils.cpp
$(info Source file: $(SRCS))

MANDATORY_FLAG = -Wall -Wcast-qual -fno-exceptions -std=c++17 -pedantic -Wextra -Wshadow -Wmissing-declarations
OPTI_FLAG = -O3
DEBUG_FLAG = -DDEBUG

COMMIT_HASH = $(shell git log -1 --pretty=format:'%h' -n 1)
COMMIT_NAME = $(shell git log -1 --pretty=%B | sed 's/ /_/g')

$(info COMMIT_HASH: $(COMMIT_HASH))
$(info COMMIT_NAME: $(COMMIT_NAME))

RELEASE_NAME = tce_$(COMMIT_NAME)_$(COMMIT_HASH).exe
$(info RELEASE_NAME: $(RELEASE_NAME))

working:
	g++ $(MANDATORY_FLAG) $(OPTI_FLAG) $(SRCS) -o tce.exe
release:
	g++ $(MANDATORY_FLAG) $(OPTI_FLAG) $(SRCS) -o $(RELEASE_NAME)
debug:
	g++ $(MANDATORY_FLAG) $(OPTI_FLAG) $(DEBUG_FLAG) $(SRCS) -o tce_d.exe
profile:
	g++ $(MANDATORY_FLAG) $(OPTI_FLAG) $(DEBUG_FLAG) $(SRCS) -g -o tce_profile.exe
