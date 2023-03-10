#ifndef DEFINITION_H_INCLUDED
#define DEFINITION_H_INCLUDED

#include <unordered_map>

#define UNDEFINED_MOVE 0

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY_GAME = 1000;
constexpr int MAX_PLY = 64;

typedef unsigned long long U64;
typedef int Move;
typedef int KillerMoves[2][MAX_PLY];
typedef int HistoryMoves[12][64];

constexpr int PIECE_NB = 12;
constexpr int OCCUPANCY_NB = 3;
constexpr int COLOR_NB = 2;

// sides to move (colors)
enum Color: int { white, black, both };

Color operator~(Color c);

enum Direction : int {
  DOWN = 8,
  UP =  -8,
  RIGHT  =  1,
  LEFT  = -1
};

Direction operator~(Direction d);

// encode pieces
enum Piece : int { P, N, B, R, Q, K, p, n, b, r, q, k, no_p };

constexpr Piece WhitePromPiece[] = {N, B, R, Q};
constexpr Piece BlackPromPiece[] = {n, b, r, q};

Piece& operator++(Piece& d);
Piece& operator--(Piece& d);


enum Square : int {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, no_sq,

  SQUARE_NB=64,
  FIRST_SQ=0
};

Square& operator++(Square& s);
Square& operator--(Square& s);

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
enum Sliding_Piece { rook, bishop };

// move type
enum Move_Type { all_moves, only_captures };


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
enum Castle_Right { wk = 1, wq = 2, bk = 4, bq = 8 };

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
  {'P', Piece::P},
  {'N', Piece::N},
  {'B', Piece::B},
  {'R', Piece::R},
  {'Q', Piece::Q},
  {'K', Piece::K},
  {'p', Piece::p},
  {'n', Piece::n},
  {'b', Piece::b},
  {'r', Piece::r},
  {'q', Piece::q},
  {'k', Piece::k},
};

const std::unordered_map<int, char> promoted_pieces = {
  {Q,  'q'},
  {N,  'n'},
  {R,  'r'},
  {B,  'b'},
  {q,  'q'},
  {n,  'n'},
  {r,  'r'},
  {b,  'b'},
  {P,  '\0'},
};

// FEN dedug positions
constexpr char const * empty_board  = "8/8/8/8/8/8/8/8 w - - ";
constexpr char const * start_position  = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
constexpr char const * tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
constexpr char const * killer_position  = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
constexpr char const * cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";


constexpr char const * perft_3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
constexpr char const * perft_4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
constexpr char const * perft_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ";
constexpr char const * perft_6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";

#endif