# TCE : The Chess Enigma

![TCE](assets/TCE.png)


## Description

TCE (The Chess Enigma) is a command-line chess engine built completely from scratch using C++ and following the UCI protocol. TCE was created out of a personal interest in chess programming and love of the game. The project explores the world of chess AI while improving C++ programming skills.

This engine represents the fusion of two captivating worlds: the strategic complexities of chess and the enigmatic art of coding. TCE is a result of curiosity, aimed to uncover the inner workings of chess algorithms, evaluation techniques, and search methods. With each line of code, TCE demystifies the chess-playing process.

## Features

TCE is an ongoing work in progress, with current features including:

#### General

- UCI protocol
- Perft testing
- Benchmarking
- Bitboard representation
- Move generation (Table / Plain magic bitboards)
- Time management (Fixed estimated time per move)

#### Search
- Iterative deepening
- NegaMax with alpha-beta pruning
- Quiescence search
- Principal variation search
- Null move pruning
- Late move reductions

#### Move ordering
- MVV-LVA
- History heuristic
- Killer heuristic

#### Transposition table 
- Zobrist hashing

#### Evaluation
- Material balance
- Piece position
- Piece mobility
- King safety
- Pawn structure (Doubled, isolated, passed)

## TODO

Features that are planned for the future, but not yet implemented:

#### General

- UCI options
- Opening book

#### Search

- Aspiration windows
- Futility pruning
- Razoring
- Singular extensions
- Static exchange evaluation
- Mate distance pruning

#### Transposition table

- Resizing

#### Evaluation

- Tapered evaluation
- NNUE

## Build

TCE is built using CMake. It has only been tested on Windows using mingw64. 

## Acknowledgment

I am deeply grateful to all the people who have generously supported me throughout this project. A special thanks goes to the members of the Stockfish discord server, whose insights and guidance have been invaluable. Your contributions have played a significant role in the development of TCE: The Chess Enigma. Your help has not only enriched the project, but also fostered a sense of community and learning. Thank you for being an integral part of this journey.
