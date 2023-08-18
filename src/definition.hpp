#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>

using U64 = uint_fast64_t;
using NodeCounter = unsigned long long;
using Score = int32_t;

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY_GAME = 1000;
constexpr int MAX_PLY_SEARCH = 64;

constexpr Score INF = 50000;
constexpr Score MATE_VALUE = 49000;
constexpr Score MATE_IN_MAX_PLY = MATE_VALUE - MAX_PLY_SEARCH;
constexpr Score DRAW_VALUE = 0;
constexpr Score NO_VALUE = 50001;

constexpr int PIECE_NB = 12;
constexpr int OCCUPANCY_NB = 3;
constexpr int COLOR_NB = 2;
constexpr int SQUARE_NB = 64;

using Move = uint32_t; 
using KillerMoves = Move[2][MAX_PLY_SEARCH];
using HistoryMoves = int[PIECE_NB][SQUARE_NB];

constexpr Move UNDEFINED_MOVE = 0;

enum Color: int { WHITE, BLACK, BOTH };

Color operator~(Color c);

enum Direction : int {
  DOWN = 8,
  UP =  -8,
  RIGHT  =  1,
  LEFT  = -1
};

Direction operator~(Direction d);

enum Piece : int { WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, NO_PIECE };

constexpr Piece WhitePromPiece[] = {Piece::WN, Piece::WB, Piece::WR, Piece::WQ};
constexpr Piece BlackPromPiece[] = {Piece::BN, Piece::BB, Piece::BR, Piece::BQ};

Piece& operator++(Piece& piece);
Piece& operator--(Piece& piece);

enum Square : int {
  A8, B8, C8, D8, E8, F8, G8, H8,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A1, B1, C1, D1, E1, F1, G1, H1, NO_SQUARE,

  FIRST_SQUARE=0,
  LAST_SQUARE=63
};

Square& operator++(Square& s);
Square& operator--(Square& s);
Square operator+(Square s, int i);
Square operator-(Square s, int i);
Square& operator+=(Square& s, int i);
Square& operator-=(Square& s, int i);


enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

File& operator++(File& s);
File& operator--(File& s);

enum Rank : int {
  RANK_8, RANK_7, RANK_6, RANK_5, RANK_4, RANK_3, RANK_2, RANK_1
};

Rank& operator++(Rank& s);
Rank& operator--(Rank& s);

template <Direction d>
Square shift(Square s) { return Square(int(s) + int(d)); };

template <Direction d>
Rank shift(Rank r) { return (d == Direction::UP) ? Rank((int)r-1) : (d == Direction::DOWN) ? Rank((int)r+1) : r; };

template <Direction d>
File shift(File f) { return (d == Direction::LEFT) ? File((int)f-1) : (d == Direction::RIGHT) ? File((int)f+1) : f; };


Square get_square(Rank r, File f);
File file_of(Square s);
Rank rank_of(Square s);

// bishop and rook
enum Sliding_Piece { ROOK, BISHOP };

// move type
enum Move_Type { ALL_MOVES, ONLY_CAPTURES };


/* castling bits binary representation
   bin     dec

  0001       1  white king can castle to the king side
  0010       2  white king can castle to the queen side
  0100       4  black king can castle to the king side
  1000       8  black king can castle to the queen side

   examples

  1111          both side can castle both direction
  1001          back king => queen side
                white king => king side

   castling rights update

                                castling        move       in   in
                                   right      update   binary   decimal
                                                            
    king & rooks didn't move:       1111    &   1111  =  1111   15
    
            white king moved:       1111    &   1100  =  1100   12
     white king's rook moved:       1111    &   1110  =  1110   14
    white queen's rook moved:       1111    &   1101  =  1101   13

            black king moved:       1111    &   0011  =  0011   3
     black king's rook moved:       1111    &   1011  =  1111   11
    black queen's rook moved:       1111    &   0111  =  1111   7

  
*/
enum Castle_Right { WOO = 1, WOOO = 2, BOO = 4, BOOO = 8 };

// castling rights update constants
constexpr int castling_rights[64] = {
  7, 15, 15, 15,  3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

constexpr char const * square_to_coordinates[] = 
{
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

// ASCII pices
constexpr char ascii_pieces[12] = { 'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k' };

const std::unordered_map<char, Piece> char_pieces {
  {'P', Piece::WP},
  {'N', Piece::WN},
  {'B', Piece::WB},
  {'R', Piece::WR},
  {'Q', Piece::WQ},
  {'K', Piece::WK},
  {'p', Piece::BP},
  {'n', Piece::BN},
  {'b', Piece::BB},
  {'r', Piece::BR},
  {'q', Piece::BQ},
  {'k', Piece::BK},
};

const std::unordered_map<int, char> promoted_pieces = {
  {Piece::WQ,  'q'},
  {Piece::WN,  'n'},
  {Piece::WR,  'r'},
  {Piece::WB,  'b'},
  {Piece::BQ,  'q'},
  {Piece::BN,  'n'},
  {Piece::BR,  'r'},
  {Piece::BB,  'b'},
  {Piece::WP,  '\0'},
};

// FEN dedug positions
const std::string empty_board = "8/8/8/8/8/8/8/8 w - - ";
const std::string start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
const std::string tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
const std::string killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
const std::string cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";


const std::string perft_3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
const std::string perft_4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
const std::string perft_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ";
const std::string perft_6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
