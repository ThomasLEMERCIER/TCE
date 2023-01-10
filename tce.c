#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>

#define U64 unsigned long long

// FEN dedug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

// Function definition ----------------------
void clear_tt();
static inline int score_move(int move);
void search_position(int depth);
static inline int evaluate();
void init_all();
static inline void generate_moves();
static inline int make_move(int move, int move_flag);
void init_magic_numbers();
U64 find_magic_number(int square, int relevant_bits, int bishop);
static inline int is_square_attacked(int square, int side);
static inline U64 get_queen_attacks(int square, U64 occupancy);
static inline U64 get_rook_attacks(int square, U64 occupancy);
static inline U64 get_bishop_attacks(int square, U64 occupancy);
void init_slider_attacks(int bishop);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
void init_leapers_attacks();
U64 rook_attacks_on_the_fly(int square, U64 occupancy);
U64 bishop_attacks_on_the_fly(int square, U64 occupancy);
U64 mask_rook_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_knight_attacks(int square);
U64 mask_pawn_attacks(int side, int square);
void print_attacked_squares(int side);
void print_move(int move);
void parse_fen(char* fen);
void print_board();
void print_bitboard(U64 bitboard);
static inline int get_lsb_index(U64 bitboard);
static inline int count_bits(U64 bitboard);
U64 generate_magic_number();
U64 get_random_U64_number();
unsigned int get_random_U32_number();
// Function definition ----------------------

// Const ------------------------------------
// board square
enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

// sides to move (colors)
enum { white, black, both };

// encode pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k};

// bishop and rook
enum { rook, bishop };

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
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

// castling rights update constants
const int castling_rights[64] = {
  7, 15, 15, 15,  3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

const char* square_to_coordinates[] = 
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
char ascii_pieces[12] = "PNBRQKpnbrqk";

// convert ASCII character pieces to encoded constants
int char_pieces[] = {
  ['P'] = P,
  ['N'] = N,
  ['B'] = B,
  ['R'] = R,
  ['Q'] = Q,
  ['K'] = K,
  ['p'] = p,
  ['n'] = n,
  ['b'] = b,
  ['r'] = r,
  ['q'] = q,
  ['k'] = k,
};

int promoted_pieces[] = {
  [Q] = 'q',
  [N] = 'n',
  [R] = 'r',
  [B] = 'b',
  [q] = 'q',
  [n] = 'n',
  [r] = 'r',
  [b] = 'b',
};

int material_score[12] = {
      100,    // white pawn score
      300,    // white knight score
      350,    // white bishop score
      500,    // white rook score
     1000,    // white queen score
    10000,    // white king score
     -100,    // black pawn score
     -300,    // black knight score
     -350,    // black bishop score
     -500,    // black rook score
    -1000,    // black queen score
   -10000,    // black king score
};

// pawn positional score
const int pawn_score[64] = 
{
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[64] = 
{
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int bishop_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,  20,   0,  10,  10,   0,  20,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0
};

// rook positional score
const int rook_score[64] =
{
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int king_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};


// mirror positional score tables for opposite side
const int mirror_score[128] = {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

/*
                          
    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/
// MVV LVA [attacker][victim]
const int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};
// Const ------------------------------------

// Timing -----------------------------------
int get_time_ms() {
  return GetTickCount();
}
// Timing -----------------------------------

// Move -------------------------------------
/* attack bits binary representation

  binary                                                                        hexidecimal

  0000 0000 0000 0000 0011 1111   source square (6 bits) (max val 63 (2^6-1))   0x3f
  0000 0000 0000 1111 1100 0000   target square (6 bits) (max val 63 (2^6-1))   0xfc0
  0000 0000 1111 0000 0000 0000   piece (4 bits) (max val 11 (2^4-1))           0xf000
  0000 1111 0000 0000 0000 0000   promoted piece (4 bits)                       0xf0000
  0001 0000 0000 0000 0000 0000   capture flag (1 bit)                          0x100000
  0010 0000 0000 0000 0000 0000   double push flag (1 bit)                      0x100000
  0100 0000 0000 0000 0000 0000   enpassant flag (1 bit)                        0x400000
  1000 0000 0000 0000 0000 0000   castling flag (1 bit)                         0x800000

*/
#define encode_move(source, target, piece, promoted, capture_f, double_f, enpassant_f, castling_f) \
  ((source))            | \
  ((target) << 6)       | \
  ((piece) << 12)       | \
  ((promoted) << 16)    | \
  ((capture_f) << 20)   | \
  ((double_f) << 21)    | \
  ((enpassant_f) << 22) | \
  ((castling_f) << 23)

#define get_move_source(move) (((move) & 0x3f))
#define get_move_target(move) (((move) & 0xfc0) >> 6)
#define get_move_piece(move) (((move) & 0xf000) >> 12)
#define get_move_promoted(move) (((move) & 0xf0000) >> 16)
#define get_move_capture_f(move) (((move) & 0x100000))
#define get_move_double_f(move) (((move) & 0x200000))
#define get_move_enpassant_f(move) (((move) & 0x400000))
#define get_move_castling_f(move) (((move) & 0x800000))

// move types
enum { all_moves, only_captures };

typedef struct
{
  // moves
  int moves[256];

  int count;
} moves;

static inline void add_move(moves *move_list, int move) {
  move_list->moves[move_list->count] = move; move_list->count++;
}
// Move -------------------------------------

// Board representation ---------------------
// bitboard for all piece types and color
U64 bitboards[12];

// occupancy for white/black/both
U64 occupancies[3];

// side to move
int side;

// en passant square
int enpassant = no_sq;

// castling rights
int castle;

U64 hash_key;

U64 repetition_table[1000];

int repetition_index;
// Board representation ---------------------

// Random Number Generator ------------------
// pseudo random number state
unsigned int random_state = 1804289383;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number() {
  // get current state
  unsigned int number = random_state;

  // xor shift algorithm
  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  // update random number state
  random_state = number;

  // renturn random number
  return number;
}

// generate 64-bit pseudo legal numbers
U64 get_random_U64_number() {
  // define 4 random numbers
  U64 n1, n2, n3, n4;

  // init random numbers keeping only the first 16 LSB
  n1 = (U64)(get_random_U32_number()) & 0xFFFF;
  n2 = (U64)(get_random_U32_number()) & 0xFFFF;
  n3 = (U64)(get_random_U32_number()) & 0xFFFF;
  n4 = (U64)(get_random_U32_number()) & 0xFFFF;

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generate_magic_number() {
  return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}
// Random Number Generator ------------------

// Macros Set/Get/Pop -----------------------
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) (get_bit((bitboard), (square)) ? (bitboard) ^= (1ULL << (square)) : 0)
// Macros Set/Get/Pop -----------------------

// Macros copy/take back --------------------
#define copy_board()                                                                            \
  U64 bitboards_copy[12], occupancies_copy[3], hash_key_copy;                                   \
  int side_copy, enpassant_copy, castle_copy;                                                   \
  memcpy(bitboards_copy, bitboards, sizeof(bitboards));                                         \
  memcpy(occupancies_copy, occupancies, sizeof(occupancies));                                   \
  side_copy = side, enpassant_copy = enpassant, castle_copy = castle, hash_key_copy = hash_key; \

#define take_back()                                                                             \
  memcpy(bitboards, bitboards_copy, sizeof(bitboards));                                         \
  memcpy(occupancies, occupancies_copy, sizeof(occupancies));                                   \
  side = side_copy, enpassant = enpassant_copy, castle = castle_copy, hash_key = hash_key_copy; \

// Macros copy/take back --------------------


// Bit Manipulation ------------------------
static inline int count_bits(U64 bitboard) {
  //bit counter
  int count = 0;

  // consecutively reset LSB
  while (bitboard) {
    // increment count
    count++;

    // reset LSB
    bitboard &= bitboard -1;
  }
  return count;
}

static inline int get_lsb_index(U64 bitboard) {
  // make sure bitboard is not 0
  // if (bitboard) {
  //   return count_bits((bitboard & -bitboard) - 1);
  // } else {
  //   return -1;
  // }
  return __builtin_ctzll(bitboard);
}
// Bit Manipulation ------------------------

// Zobrist key ------------------------------

U64 piece_keys[12][64];

U64 side_key;

U64 enpassant_keys[64];

U64 castle_keys[64];

// init random hash keys
void init_random_keys()
{
  // update pseudo random number state
  random_state = 1804289383;

  // loop over piece codes
  for (int piece = P; piece <= k; piece++)
  {
    // loop over board squares
    for (int square = 0; square < 64; square++)
      // init random piece keys
      piece_keys[piece][square] = get_random_U64_number();
  }
  
  // loop over board squares
  for (int square = 0; square < 64; square++)
    // init random enpassant keys
    enpassant_keys[square] = get_random_U64_number();
  
  // loop over castling keys
  for (int index = 0; index < 16; index++)
    // init castling keys
    castle_keys[index] = get_random_U64_number();
      
  // init random side key
  side_key = get_random_U64_number();
}

U64 generate_hash_key()
{
  // final hash key
  U64 final_key = 0ULL;
  
  // temp piece bitboard copy
  U64 bitboard;
  
  // loop over piece bitboards
  for (int piece = P; piece <= k; piece++)
  {
    // init piece bitboard copy
    bitboard = bitboards[piece];
    
    // loop over the pieces within a bitboard
    while (bitboard)
    {
      // init square occupied by the piece
      int square = get_lsb_index(bitboard);
      
      // hash piece
      final_key ^= piece_keys[piece][square];
      
      // pop LS1B
      pop_bit(bitboard, square);
    }
  }
  
  // if enpassant square is on board
  if (enpassant != no_sq)
      // hash enpassant
      final_key ^= enpassant_keys[enpassant];
  
  // hash castling rights
  final_key ^= castle_keys[castle];
  
  // hash the side only if black is to move
  if (side == black) final_key ^= side_key;
  
  // return generated hash key
  return final_key;
}
// Zobrist key ------------------------------

// Input / Output ---------------------------
void print_bitboard(U64 bitboard) { 
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int square = 8 * rank + file;

      if (!file) {
        printf("  %d ", 8-rank);
      }

      printf(" %d", (bitboard & (1ULL << square)) ? 1 : 0);
    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n\n");
  printf("     Bitboard: %llud\n\n", bitboard);
}

void print_board() {
  for (int rank = 0; rank < 8; rank++) {
    for (int file =0; file < 8; file++) {
      int square = 8 * rank + file;

      if (!file) {
        printf("  %d ", 8-rank);
      }

      int piece = -1;

      for (int bb_piece = P; bb_piece <= k; bb_piece++) {
        if (get_bit(bitboards[bb_piece], square)) {
          piece = bb_piece; break;
        }
      }

      printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n\n");
  printf("     Side:     %s\n", (!side) ? "white" : "black");
  printf("     En Passant:  %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
  printf("     Casting:   %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                       (castle & wq) ? 'Q' : '-',
                                       (castle & bk) ? 'k' : '-',
                                       (castle & bq) ? 'q' : '-');
  printf("   Hash Key:  %llx\n\n", hash_key);
}

void parse_fen(char* fen) {
  // reset board position and state variables
  memset(bitboards, 0ULL, sizeof(bitboards));
  memset(occupancies, 0ULL, sizeof(occupancies));
  side = 0;
  enpassant = no_sq;
  castle = 0;

  for (int rank = 0; rank < 8; rank++) {
    for (int file =0; file < 8; file++) {
      int square = 8 * rank + file;

      // matching piece
      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        int piece = char_pieces[*fen];
        set_bit(bitboards[piece], square);
        fen++;
      }
      // matching empty squares
      else if ((*fen >= '0' && *fen <= '9')) {
        int offset = *fen - '0' - 1;
        file += offset;
        fen++;
      }
      // match rank sep
      if (*fen == '/') {
        fen++;
      }
    }
  }

  // skip whitespace to parse side to move
  fen++;
  (*fen == 'w') ? (side = white) : (side = black);

  // skip color and whitespace to parse castling right
  fen += 2;
  while (*fen != ' ') {
    switch (*fen)
    {
      case 'K': castle |= wk; break;
      case 'Q': castle |= wq; break;
      case 'k': castle |= bk; break;
      case 'q': castle |= bq; break;
      default: break;
    }
    fen++;
  }

  // skip whitespace to parse en passant square 
  fen++;
  if (*fen == '-') {
    enpassant = no_sq;
  }
  else {
    int file = fen[0] - 'a';
    int rank = 8 - (fen[1] - '0');
    enpassant = 8 * rank + file;
  }
  // printf("fen: '%s'\n", fen);

  // loop over white pieces bitboards
  for (int piece = P; piece <= K; piece++)
    occupancies[white] |= bitboards[piece];
  // loop over black pieces bitboards
  for (int piece = p; piece <= k; piece++)
    occupancies[black] |= bitboards[piece];

  occupancies[both] |= occupancies[white];
  occupancies[both] |= occupancies[black];

  hash_key = generate_hash_key();
  repetition_index = 0;
  repetition_table[repetition_index] = hash_key;
}

void print_attacked_squares(int side) {  
  for (int rank = 0; rank < 8; rank++) {
    for (int file =0; file < 8; file++) {
      int square = 8 * rank + file;

      if (!file) {
        printf("  %d ", 8-rank);
      }

      printf(" %d", (is_square_attacked(square, side)) ? 1: 0);

    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n\n");
}

void print_move(int move) {
  if (get_move_promoted(move))
    printf("%s%s%c", square_to_coordinates[get_move_source(move)],
                      square_to_coordinates[get_move_target(move)],
                      promoted_pieces[get_move_promoted(move)]);
  else
    printf("%s%s", square_to_coordinates[get_move_source(move)],
                      square_to_coordinates[get_move_target(move)]);
}

void print_move_list(moves *move_list) {
  printf("  move    piece   double   enpassant   castling\n\n");
  for (int move_count = 0; move_count < move_list->count; move_count++)
  {
    int move = move_list->moves[move_count];

    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted = get_move_promoted(move);
    int capture_f = get_move_capture_f(move);
    int double_f = get_move_double_f(move);
    int enpassant_f = get_move_enpassant_f(move);
    int castling_f = get_move_castling_f(move);

    if (promoted) {
      if (capture_f)
        printf("  %sx%s%c  %c       %d        %d           %d\n", square_to_coordinates[source_square],
                                                                  square_to_coordinates[target_square],
                                                                  promoted_pieces[promoted],
                                                                  ascii_pieces[piece],
                                                                  (double_f) ? 1 : 0,
                                                                  (enpassant_f) ? 1 : 0,
                                                                  (castling_f) ? 1 : 0);
      else
        printf("  %s%s%c   %c       %d        %d           %d\n", square_to_coordinates[source_square],
                                                                  square_to_coordinates[target_square],
                                                                  promoted_pieces[promoted],
                                                                  ascii_pieces[piece],
                                                                  (double_f) ? 1 : 0,
                                                                  (enpassant_f) ? 1 : 0,
                                                                  (castling_f) ? 1 : 0);
    }
    else {
      if (capture_f)
        printf("  %sx%s   %c       %d        %d           %d\n", square_to_coordinates[source_square],
                                                                square_to_coordinates[target_square],
                                                                ascii_pieces[piece],
                                                                (double_f) ? 1 : 0,
                                                                (enpassant_f) ? 1 : 0,
                                                                (castling_f) ? 1 : 0);
      else
        printf("  %s%s    %c       %d        %d           %d\n", square_to_coordinates[get_move_source(move)],
                                                                 square_to_coordinates[get_move_target(move)],
                                                                 ascii_pieces[piece],
                                                                 (double_f) ? 1 : 0,
                                                                 (enpassant_f) ? 1 : 0,
                                                                 (castling_f) ? 1 : 0);
    }
  }
  printf("\n\n Total number of moves: %d\n\n", move_list->count);
}

void print_move_scores(moves *move_list) {
  printf("    Move Scores\n");
  for (int move_count = 0; move_count < move_list->count; move_count++) {
      printf("    move: ");
      print_move(move_list->moves[move_count]);
      printf(",  score: %d\n", score_move(move_list->moves[move_count]));
    }
}
// Input / Output ---------------------------

// UCI --------------------------------------
// exit from engine flag
int quit = 0;

// UCI "movestogo" command moves counter
int movestogo = 30;

// UCI "movetime" command time counter
int movetime = -1;

// UCI "time" command holder (ms)
int time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;

int input_waiting()
{
    #ifndef WIN32
        printf("WIN32\n");
        fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
    #else
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

    #endif
}

// read GUI/user input
void read_input()
{
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
static void communicate() {
	// if time is up break here
    if(timeset == 1 && get_time_ms() > stoptime) {

    printf("Stopping\n");
		// tell engine to stop calculating
		stopped = 1;
	}
	
    // read GUI input
	read_input();
}

int parse_move(char *move_string) {
  // generate moves
  moves move_list[1];
  generate_moves(move_list);

  // parse squares
  int source_square = (move_string[0] - 'a') + 8 * (8 - (move_string[1] - '0'));
  int target_square = (move_string[2] - 'a') + 8 * (8 - (move_string[3] - '0'));

  for (int move_count = 0; move_count < move_list->count; move_count++) {
    int move = move_list->moves[move_count];

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

int parse_position(char *command) {
  // shift pointer at next token
  command += 9;

  // init pointer to current character in the command string
  char *current_char = command;

  // parse UCI "startpos" command
  if (strncmp(command, "startpos", 8) == 0) {
    // init chess board with start position
    parse_fen(start_position);
  }
  // parse UCI "fen" command
  else {
    // make sure "fen" command is available in the command string
    current_char = strstr(command, "fen");

    if (current_char == NULL)
      parse_fen(start_position);
    else {
      current_char += 4;

      parse_fen(current_char);
    }
  }

  current_char = strstr(command, "moves");

  // moves available
  if (current_char != NULL) {
    // shift pointer to the next token
    current_char += 6;

    // loop over moves
    while (*current_char) {

      int move = parse_move(current_char);

      if (move == 0)
        break;

      make_move(move, all_moves);
      repetition_index++;
      repetition_table[repetition_index] = hash_key;

      // move pointer to end of current move
      while (*current_char && *current_char != ' ') current_char++;

      // move pointer to the next move
      current_char++;
    }
  }
}

void parse_go(char *command)
{
  // init parameters
  int depth = -1;

  // init argument
  char *argument = NULL;

  // infinite search
  if ((argument = strstr(command,"infinite"))) {}

  // match UCI "binc" command
  if ((argument = strstr(command,"binc")) && side == black)
    // parse black time increment
    inc = atoi(argument + 5);

  // match UCI "winc" command
  if ((argument = strstr(command,"winc")) && side == white)
    // parse white time increment
    inc = atoi(argument + 5);

  // match UCI "wtime" command
  if ((argument = strstr(command,"wtime")) && side == white)
    // parse white time limit
    time = atoi(argument + 6);

  // match UCI "btime" command
  if ((argument = strstr(command,"btime")) && side == black)
    // parse black time limit
    time = atoi(argument + 6);

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
    time = movetime;

    // set moves to go to 1
    movestogo = 1;
  }

  // init start time
  starttime = get_time_ms();

  // init search depth
  depth = depth;

  // if time control is available
  if(time != -1)
  {
    // flag we're playing with time control
    timeset = 1;

    // set up timing
    time /= movestogo;
    time -= 50;
    stoptime = starttime + time + inc;
  }

  // if depth is not available
  if(depth == -1)
    // set depth to 64 plies (takes ages to complete...)
    depth = 64;

  // print debug info
  printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
  time, starttime, stoptime, depth, timeset);

  // search position
  search_position(depth);
}

void uci_loop() {
  // reset STDIN & STDOUT buffers
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  // define user / GUI input buffer
  char input[2000];

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
      clear_tt();
      repetition_index = 0;
      parse_position("position startpos");
    }

    // parse UCI "position" command
    else if (strncmp(input, "position", 8) == 0) {
      clear_tt();
      parse_position(input);
    }

    // parse UCI "go" command
    else if (strncmp(input, "go", 2) == 0)
      parse_go(input);

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
// UCI --------------------------------------

// Attacks ----------------------------------
/*
      not A file
  8  0 1 1 1 1 1 1 1
  7  0 1 1 1 1 1 1 1
  6  0 1 1 1 1 1 1 1
  5  0 1 1 1 1 1 1 1
  4  0 1 1 1 1 1 1 1
  3  0 1 1 1 1 1 1 1
  2  0 1 1 1 1 1 1 1
  1  0 1 1 1 1 1 1 1

     a b c d e f g h

      not H file
  8  1 1 1 1 1 1 1 0
  7  1 1 1 1 1 1 1 0
  6  1 1 1 1 1 1 1 0
  5  1 1 1 1 1 1 1 0
  4  1 1 1 1 1 1 1 0
  3  1 1 1 1 1 1 1 0
  2  1 1 1 1 1 1 1 0
  1  1 1 1 1 1 1 1 0

     a b c d e f g h

      not AB file
  8  0 0 1 1 1 1 1 1
  7  0 0 1 1 1 1 1 1
  6  0 0 1 1 1 1 1 1
  5  0 0 1 1 1 1 1 1
  4  0 0 1 1 1 1 1 1
  3  0 0 1 1 1 1 1 1
  2  0 0 1 1 1 1 1 1
  1  0 0 1 1 1 1 1 1

     a b c d e f g h

      not HG file
  8  1 1 1 1 1 1 0 0
  7  1 1 1 1 1 1 0 0
  6  1 1 1 1 1 1 0 0
  5  1 1 1 1 1 1 0 0
  4  1 1 1 1 1 1 0 0
  3  1 1 1 1 1 1 0 0
  2  1 1 1 1 1 1 0 0
  1  1 1 1 1 1 1 0 0

     a b c d e f g h
*/

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_ab_file = 18229723555195321596ULL;
const U64 not_hg_file = 4557430888798830399ULL;

// relevant occupancy bit count for every square on board
const int bishop_relevant_bits[64] = 
{
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

const int rook_relevant_bits[64] = 
{
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

// rook magic numbers [square]
const U64 rook_magic_numbers[64] = 
{
  0x8a80104000800020ULL,
  0x140002000100040ULL,
  0x2801880a0017001ULL,
  0x100081001000420ULL,
  0x200020010080420ULL,
  0x3001c0002010008ULL,
  0x8480008002000100ULL,
  0x2080088004402900ULL,
  0x800098204000ULL,
  0x2024401000200040ULL,
  0x100802000801000ULL,
  0x120800800801000ULL,
  0x208808088000400ULL,
  0x2802200800400ULL,
  0x2200800100020080ULL,
  0x801000060821100ULL,
  0x80044006422000ULL,
  0x100808020004000ULL,
  0x12108a0010204200ULL,
  0x140848010000802ULL,
  0x481828014002800ULL,
  0x8094004002004100ULL,
  0x4010040010010802ULL,
  0x20008806104ULL,
  0x100400080208000ULL,
  0x2040002120081000ULL,
  0x21200680100081ULL,
  0x20100080080080ULL,
  0x2000a00200410ULL,
  0x20080800400ULL,
  0x80088400100102ULL,
  0x80004600042881ULL,
  0x4040008040800020ULL,
  0x440003000200801ULL,
  0x4200011004500ULL,
  0x188020010100100ULL,
  0x14800401802800ULL,
  0x2080040080800200ULL,
  0x124080204001001ULL,
  0x200046502000484ULL,
  0x480400080088020ULL,
  0x1000422010034000ULL,
  0x30200100110040ULL,
  0x100021010009ULL,
  0x2002080100110004ULL,
  0x202008004008002ULL,
  0x20020004010100ULL,
  0x2048440040820001ULL,
  0x101002200408200ULL,
  0x40802000401080ULL,
  0x4008142004410100ULL,
  0x2060820c0120200ULL,
  0x1001004080100ULL,
  0x20c020080040080ULL,
  0x2935610830022400ULL,
  0x44440041009200ULL,
  0x280001040802101ULL,
  0x2100190040002085ULL,
  0x80c0084100102001ULL,
  0x4024081001000421ULL,
  0x20030a0244872ULL,
  0x12001008414402ULL,
  0x2006104900a0804ULL,
  0x1004081002402ULL
};

// bishop magic numbers [square]
const U64 bishop_magic_numbers[64] = 
{
  0x40040844404084ULL,
  0x2004208a004208ULL,
  0x10190041080202ULL,
  0x108060845042010ULL,
  0x581104180800210ULL,
  0x2112080446200010ULL,
  0x1080820820060210ULL,
  0x3c0808410220200ULL,
  0x4050404440404ULL,
  0x21001420088ULL,
  0x24d0080801082102ULL,
  0x1020a0a020400ULL,
  0x40308200402ULL,
  0x4011002100800ULL,
  0x401484104104005ULL,
  0x801010402020200ULL,
  0x400210c3880100ULL,
  0x404022024108200ULL,
  0x810018200204102ULL,
  0x4002801a02003ULL,
  0x85040820080400ULL,
  0x810102c808880400ULL,
  0xe900410884800ULL,
  0x8002020480840102ULL,
  0x220200865090201ULL,
  0x2010100a02021202ULL,
  0x152048408022401ULL,
  0x20080002081110ULL,
  0x4001001021004000ULL,
  0x800040400a011002ULL,
  0xe4004081011002ULL,
  0x1c004001012080ULL,
  0x8004200962a00220ULL,
  0x8422100208500202ULL,
  0x2000402200300c08ULL,
  0x8646020080080080ULL,
  0x80020a0200100808ULL,
  0x2010004880111000ULL,
  0x623000a080011400ULL,
  0x42008c0340209202ULL,
  0x209188240001000ULL,
  0x400408a884001800ULL,
  0x110400a6080400ULL,
  0x1840060a44020800ULL,
  0x90080104000041ULL,
  0x201011000808101ULL,
  0x1a2208080504f080ULL,
  0x8012020600211212ULL,
  0x500861011240000ULL,
  0x180806108200800ULL,
  0x4000020e01040044ULL,
  0x300000261044000aULL,
  0x802241102020002ULL,
  0x20906061210001ULL,
  0x5a84841004010310ULL,
  0x4010801011c04ULL,
  0xa010109502200ULL,
  0x4a02012000ULL,
  0x500201010098b028ULL,
  0x8040002811040900ULL,
  0x28000010020204ULL,
  0x6000020202d0240ULL,
  0x8918844842082200ULL,
  0x4010011029020020ULL
};

// pawn attacks table [side][square]
U64 pawn_attacks[2][64];
// knight attacks table [square]
U64 knight_attacks[64];
// king attacks table [square]
U64 king_attacks[64];
// bishop attack masks [square]
U64 bishop_masks[64];
// rook attack masks [square]
U64 rook_masks[64];
// bishop attack table [square][occupancies]
U64 bishop_attacks[64][512];
// rook attack table [square][occupancies]
U64 rook_attacks[64][4096];

U64 mask_pawn_attacks(int side, int square) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // piece bitboard
  U64 bitboard = 0ULL;

  // set piece on board
  set_bit(bitboard, square);

  // white pawns
  if (!side) {
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
  } 
  // black pawns
  else {
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
  }

  return attacks;
}

U64 mask_knight_attacks(int square) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // piece bitboard
  U64 bitboard = 0ULL;

  // set piece on board
  set_bit(bitboard, square);

  if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
  if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
  if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
  if ((bitboard >> 6) & not_ab_file) attacks |= (bitboard >> 6);

  if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
  if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
  if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
  if ((bitboard << 6) & not_hg_file) attacks |= (bitboard << 6);

  return attacks;
}

U64 mask_king_attacks(int square) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // piece bitboard
  U64 bitboard = 0ULL;

  // set piece on board
  set_bit(bitboard, square);

  if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
  if (bitboard >> 8) attacks |= (bitboard >> 8);
  if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
  if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);

  if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
  if (bitboard << 8) attacks |= (bitboard << 8);
  if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
  if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);

  return attacks;
}

U64 mask_bishop_attacks(int square) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // init ranks & files
  int r, f;

  // init target rank and file
  int tr = square / 8;
  int tf = square % 8;

  // mask relevant bishop occupancy bits
  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++) attacks |= (1ULL << (r * 8 + f));
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--) attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--) attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++) attacks |= (1ULL << (r * 8 + f));

  return attacks;
}

U64 mask_rook_attacks(int square) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // init ranks & files
  int r, f;

  // init target rank and file
  int tr = square / 8;
  int tf = square % 8;

  // mask relevant rook occupancy bits
  for (r = tr + 1; r < 7; r++) attacks |= (1ULL << (r * 8 + tf));
  for (r = tr - 1; r > 0; r--) attacks |= (1ULL << (r * 8 + tf));
  for (f = tf - 1; f > 0; f--) attacks |= (1ULL << (tr * 8 + f));
  for (f = tf + 1; f < 7; f++) attacks |= (1ULL << (tr * 8 + f));

  return attacks;
}

U64 bishop_attacks_on_the_fly(int square, U64 occupancy) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // init ranks & files
  int r, f;

  // init target rank and file
  int tr = square / 8;
  int tf = square % 8;

  // mask relevant bishop occupancy bits
  for (r = tr + 1, f = tf + 1; r < 8 && f < 8; r++, f++) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & occupancy) break;
  }
  for (r = tr + 1, f = tf - 1; r < 8 && f > -1; r++, f--) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & occupancy) break;
  }
  for (r = tr - 1, f = tf - 1; r > -1 && f > -1; r--, f--) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & occupancy) break;
  }
  for (r = tr - 1, f = tf + 1; r > -1 && f < 8; r--, f++){
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & occupancy) break;
  }

  return attacks;
}

U64 rook_attacks_on_the_fly(int square, U64 occupancy) {
  // result attacks bitboard
  U64 attacks = 0ULL;

  // init ranks & files
  int r, f;

  // init target rank and file
  int tr = square / 8;
  int tf = square % 8;

  // mask relevant rook occupancy bits
  for (r = tr + 1; r < 8; r++) {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & occupancy) break;
  }
  for (r = tr - 1; r > -1; r--) {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & occupancy) break;
  }
  for (f = tf - 1; f > -1; f--) {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & occupancy) break;
  }
  for (f = tf + 1; f < 8; f++) {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & occupancy) break;
  }

  return attacks;
}

void init_leapers_attacks() {
  // loop over 64 board squares
  for (int square=0; square < 64; square++) {
    // init pawn attacks
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);

    // init knight attacks
    knight_attacks[square] = mask_knight_attacks(square);

    // init king attacks
    king_attacks[square] = mask_king_attacks(square);

  }
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  // occupancy map
  U64 occupancy = 0ULL;

  // loop over the range of bits within attack mask
  for (int count = 0; count < bits_in_mask; count++) {
    // get LSB index of attack mask
    int square = get_lsb_index(attack_mask);

    // pop LSB in attack mask
    pop_bit(attack_mask, square);

    // make sure occupancy is on board
    if (index & (1 << count))
      occupancy |= (1ULL << square);
  }

  return occupancy;
}

void init_slider_attacks(int bishop) {
  // loop over 64 board squares
  for (int square = 0; square < 64; square++) {
    // init bishop & rook masks
    bishop ? (bishop_masks[square] = mask_bishop_attacks(square)) : (rook_masks[square] = mask_rook_attacks(square));

    // init current mask
    U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

    // init relevant occupancy bit count
    int relevant_bits_count = bishop? bishop_relevant_bits[square] : rook_relevant_bits[square];

    // int occupancy indices
    int occupancy_indices = (1 << relevant_bits_count);

    // loop over occupancy indices
    for (int index = 0; index < occupancy_indices; index++) {
      if (bishop) {
        // init current occupancy variation
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        // init magic index
        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);

        // init bishop attacks
        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
      }
      else {
        // init current occupancy variation
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        // init magic index
        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);

        // init bishop attacks
        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
      }
    }
  }
}

static inline U64 get_bishop_attacks(int square, U64 occupancy) {
  occupancy &= bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= 64 - bishop_relevant_bits[square];
  return bishop_attacks[square][occupancy];
}

static inline U64 get_rook_attacks(int square, U64 occupancy) {
  occupancy &= rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= 64 - rook_relevant_bits[square];
  return rook_attacks[square][occupancy];
}

static inline U64 get_queen_attacks(int square, U64 occupancy) {
  U64 bishop_occupancy = occupancy;
  U64 rook_occupancy = occupancy;

  rook_occupancy &= rook_masks[square];
  rook_occupancy *= rook_magic_numbers[square];
  rook_occupancy >>= 64 - rook_relevant_bits[square];

  bishop_occupancy &= bishop_masks[square];
  bishop_occupancy *= bishop_magic_numbers[square];
  bishop_occupancy >>= 64 - bishop_relevant_bits[square];

  return rook_attacks[square][rook_occupancy] | bishop_attacks[square][bishop_occupancy];
}

// is the square attacked by the side
static int is_square_attacked(int square, int side) {
  // using symetry of attack pattern

  // white pawn attack
  if ((side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;

  // black pawn attack
  if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

  // knight attack
  if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;

  // bishop attack
  if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;

  // bishop attack
  if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;

  // queen attack
  if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;

  // king attack
  if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

  return 0;
}
// Attacks ----------------------------------

// Magics -----------------------------------
U64 find_magic_number(int square, int relevant_bits, int bishop) {
  // init occupancies
  U64 occupancies[4096];

  // init attack tables
  U64 attacks[4096];
  
  // init used attacks
  U64 used_attacks[4096];

  // init attack_mask for a current piece
  U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  // init occupancy indices
  int occupancy_indices = 1 << relevant_bits;

  // loop over occupancy indices
  for (int index = 0; index < occupancy_indices; index++) {
    // inint occupancies
    occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);

    // init attacks
    attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) : 
                              rook_attacks_on_the_fly(square, occupancies[index]);
  }

  // test magic numbers loop
  for (int random_count = 0; random_count < 100000000; random_count++) {
    //generate magic number candidate
    U64 magic_number = generate_magic_number();

    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
    // printf(" %d\n", magic_number);

    // init used attacks
    memset(used_attacks, 0ULL, sizeof(used_attacks));

    // init index & fail flag
    int index, fail;

    // test magic index
    for (index = 0, fail = 0; !fail && index < occupancy_indices; index++) {
      // init magic index

      int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
      // printf("Testing on %d, %d : used_attack %d,  attacks %d\n", magic_index, index, used_attacks[magic_index], attacks[index]);

      // on empty index available
      if (used_attacks[magic_index] == 0ULL)
        used_attacks[magic_index] = attacks[index];

      // otherwise
      else if  (used_attacks[magic_index] != attacks[index]) {
        fail = 1;
      }
    }

    if (!fail)
      return magic_number;
  }
  printf("   Magic number fails!");
  return 0ULL;
}

void init_magic_numbers() {
  // loop over 64 board squares
  for (int square = 0; square < 64; square++)
    // print rook magic numbers
    printf(" 0x%llxULL,\n", find_magic_number(square, rook_relevant_bits[square], rook));


  // loop over 64 board squares
  for (int square = 0; square < 64; square++)
    // print bishop magic numbers
    printf(" 0x%llxULL,\n", find_magic_number(square, bishop_relevant_bits[square], bishop));
}
// Magics -----------------------------------

// Move Generation --------------------------
static inline void generate_moves(moves *move_list) {
  // init move count
  move_list->count = 0;

  // init source and target squares
  int source_square, target_square;

  // define current piece's bitboard copy & it's attack
  U64 bitboard, attacks;

  // loop over all bitboards
  for (int piece = P; piece <= k; piece++) {
    // init piece bitboard copy
    bitboard = bitboards[piece];

    // generate white pawns & white king castling moves
    if (side == white) {
      // white pawn bitboard index
      if (piece == P) {
        // loop over white pawns
        while  (bitboard) {
          // init source square
          source_square = get_lsb_index(bitboard);

          // init target square
          target_square = source_square - 8;

          // quiet pawn move
          if (!(target_square < a8) && !get_bit(occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= a7 && source_square <= h7) {
              // addd move into a move list            
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
            }
            else {
              // one square pawn move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

              // double square pawn move
              target_square = source_square - 16;
              if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square))
                add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 1, 0, 0));

            }
          }

          // init pawn attacks bitboard
          attacks = pawn_attacks[side][source_square] & occupancies[black];

          // generate pawn captures
          while (attacks)
          {
            // init target square
            target_square = get_lsb_index(attacks);
            
            if (source_square >= a7 && source_square <= h7) {
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
            }
            else
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (enpassant != no_sq)
          {
              // lookup pawn attacks and bitwise AND with enpassant square (bit)
              U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
              
              // make sure enpassant capture available
              if (enpassant_attacks)
              {
                  // init enpassant capture target square
                  int target_enpassant = get_lsb_index(enpassant_attacks);
                  add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
              }
          }

          // pop lsb from piece bitboard copy
          pop_bit(bitboard, source_square);
        }
        // white king bitboard index
      }
      else if (piece == K) {
        // king side castling is available
        if (castle & wk) {
          // make sure square betwwen king and king's rook are empty
          if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black)) {
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
            }
          }
        }
        if (castle & wq) {
          // make sure square betwwen king and queen's rook are empty
          if (!get_bit(occupancies[both], b1) && !get_bit(occupancies[both], c1)& !get_bit(occupancies[both], d1))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(d1, black) && !is_square_attacked(e1, black)) {
              add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    }
    else {
      // black pawn bitboard index
      if (piece == p) {
        // loop over black pawns
        while  (bitboard) {
          // init source square
          source_square = get_lsb_index(bitboard);

          // init target square
          target_square = source_square + 8;

          if (!(target_square > h1) && !get_bit(occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= a2 && source_square <= h2) {
              // addd move into a move list
              add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
            }
            else {
              // one square pawn move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

              // double square pawn move
              target_square = source_square + 16;
              if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square))
                add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 1, 0, 0));
            }
          }

          // init pawn attacks bitboard
          attacks = pawn_attacks[side][source_square] & occupancies[white];

          // generate pawn captures
          while (attacks)
          {
            // init target square
            target_square = get_lsb_index(attacks);
            
            if (source_square >= a2 && source_square <= h2) {
              add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
            }
            else
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (enpassant != no_sq)
          {
              // lookup pawn attacks and bitwise AND with enpassant square (bit)
              U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
              
              // make sure enpassant capture available
              if (enpassant_attacks)
              {
                  // init enpassant capture target square
                  int target_enpassant = get_lsb_index(enpassant_attacks);
                  add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
              }
          }
          
          // pop lsb from piece bitboard copy
          pop_bit(bitboard, source_square);
        }
      }
      else if (piece == k) {
        // king side castling is available
        if (castle & bk) {
          // make sure square betwwen king and king's rook are empty
          if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white)) {
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
            }
          }
        }
        if (castle & bq) {
          // make sure square betwwen king and queen's rook are empty
          if (!get_bit(occupancies[both], b8) && !get_bit(occupancies[both], c8)& !get_bit(occupancies[both], d8))
          {
            // make sure king and moving square are not attack
            if (!is_square_attacked(d8, white) && !is_square_attacked(e8, white)) {
              add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
            }
          }
        }
      }
    }

    // generate knight moves
    if ((side == white) ? piece == N : piece == n) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate bishop moves
    if ((side == white) ? piece == B : piece == b) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate rook moves
    if ((side == white) ? piece == R : piece == r) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate queen moves
    if ((side == white) ? piece == Q : piece == q) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate king moves
    if ((side == white) ? piece == K : piece == k) {
      while  (bitboard) {
        // init source square
        source_square = get_lsb_index(bitboard);

        // init pawn attacks bitboard
        attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // generate pawn captures
        while (attacks)
        {
          // init target square
          target_square = get_lsb_index(attacks);

          if (!get_bit((side == white) ? occupancies[black] : occupancies[white], target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        // pop lsb from piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

  }
}

static inline int make_move(int move, int move_flag) {
  // all moves
  if (move_flag == all_moves) {
    // preserve board state
    copy_board();

    // parse move
    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted = get_move_promoted(move);
    int capture_f = get_move_capture_f(move);
    int double_f = get_move_double_f(move);
    int enpassant_f = get_move_enpassant_f(move);
    int castling_f = get_move_castling_f(move);

    // move piece
    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);
    hash_key ^= piece_keys[piece][source_square];
    hash_key ^= piece_keys[piece][target_square];

    // handle capture
    if (capture_f) {
      int start_piece = (side == white) ? p : P;
      int end_piece = (side == white) ? k : K;

      for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
        if (get_bit(bitboards[bb_piece], target_square)) {
          pop_bit(bitboards[bb_piece], target_square);
          hash_key ^= piece_keys[bb_piece][target_square];
          break;
        }
      }
    }

    // handle pawn promotion
    if (promoted) {
      set_bit(bitboards[promoted], target_square);
      pop_bit(bitboards[piece], target_square);
      hash_key ^= piece_keys[promoted][target_square];
      hash_key ^= piece_keys[piece][target_square];
    }

    // handle en passant
    if (enpassant_f) {
      if (side == white) {
        pop_bit(bitboards[p], target_square + 8);
        hash_key ^= piece_keys[p][target_square + 8];
      }
      else {
        pop_bit(bitboards[P], target_square - 8);
        hash_key ^= piece_keys[P][target_square - 8];
      }
 
    }

    // reset enpassant square
    if (enpassant != no_sq) {
      hash_key ^= enpassant_keys[enpassant];
      enpassant = no_sq;
    }

    // handle double pawn move
    if (double_f) {
      if (side == white) {
        enpassant = target_square + 8;
      }
      else {
        enpassant = target_square - 8;
      }
      hash_key ^= enpassant_keys[enpassant];
    }

    // handle castle
    if (castling_f) {
      // move rook
      switch (target_square)
      {
        case g1:
          set_bit(bitboards[R], f1);
          pop_bit(bitboards[R], h1);
          hash_key ^= piece_keys[R][f1];
          hash_key ^= piece_keys[R][h1];
          break;
        case c1:
          set_bit(bitboards[R], d1);
          pop_bit(bitboards[R], a1);
          hash_key ^= piece_keys[R][d1];
          hash_key ^= piece_keys[R][a1];
          break;
        case g8:
          set_bit(bitboards[r], f8);
          pop_bit(bitboards[r], h8);
          hash_key ^= piece_keys[r][f8];
          hash_key ^= piece_keys[r][h8];
          break;
        case c8:
          set_bit(bitboards[r], d8);
          pop_bit(bitboards[r], a8);
          hash_key ^= piece_keys[r][d8];
          hash_key ^= piece_keys[r][a8];
          break;
        }
    }

    // castling rights update
    hash_key ^= castle_keys[castle];
    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];
    hash_key ^= castle_keys[castle];

    // reset occupancies
    memset(occupancies, 0ULL, sizeof(occupancies));

    // loop over white pieces bitboards
    for (int bb_piece = P; bb_piece <= K; bb_piece++)
      occupancies[white] |= bitboards[bb_piece];
    // loop over black pieces bitboards
    for (int bb_piece = p; bb_piece <= k; bb_piece++)
      occupancies[black] |= bitboards[bb_piece];

    // update both sides occupancy table
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

    // change side
    side ^= 1;
    hash_key ^= side_key;

    // make sure that king is not in check
    if (is_square_attacked((side == white) ? get_lsb_index(bitboards[k]) : get_lsb_index(bitboards[K]), side)) {
      // move is illegal, take it back
      take_back();

      return 0; 
    }
    else
      // return legal move
      return 1;
  }
  // capture moves
  else {
    // check if the move is a capture
    if (get_move_capture_f(move)) {
      make_move(move, all_moves);
    }
    else
      // don't make move
      return 0;
  }
}
// Move Generation --------------------------

// Init All ----------------------------------
void init_all() {
  // init leaper pieces attacks
  init_leapers_attacks();

  init_slider_attacks(bishop);
  init_slider_attacks(rook);

  init_random_keys();
}
// Init All ----------------------------------

// perft drive -------------------------------
long nodes;

static inline void perft_driver(int depth) {
  // reccursion espace condition
  if (depth == 0) {
    nodes++;
    return;
  }
  else {
    // generate moves
    moves move_list[1];
    generate_moves(move_list);

    // loop over generated moves
    for (int move_count = 0; move_count < move_list->count; move_count++) {
      //  preserve board state
      copy_board();

      if (!make_move(move_list->moves[move_count], all_moves)) 
        continue;

      perft_driver(depth - 1);

      // restore board
      take_back();
    }
  }
}

void perft_test(int depth) {
  printf("\n\n Performance test\n\n");

  long start = get_time_ms();

  // generate moves
  moves move_list[1];
  generate_moves(move_list);

  // loop over generated moves
  for (int move_count = 0; move_count < move_list->count; move_count++) {
    //  preserve board state
    copy_board();

    if (!make_move(move_list->moves[move_count], all_moves)) 
      continue;

    // cummulative nodes
    long cummulative_nodes = nodes;

    perft_driver(depth - 1);

    // old nodes
    long old_nodes = nodes - cummulative_nodes;

    // restore board
    take_back();
    printf("    move: %s%s%c  nodes : %ld\n", square_to_coordinates[get_move_source(move_list->moves[move_count])],
                                              square_to_coordinates[get_move_target(move_list->moves[move_count])],
                                              (get_move_promoted(move_list->moves[move_count])) ? ascii_pieces[get_move_promoted(move_list->moves[move_count])] : ' ',
                                              old_nodes);
  }

  printf("\n    Depth: %d", depth);
  printf("\n    Nodes: %ld", nodes);
  printf("\n    Time: %ldms\n\n", get_time_ms() - start);

}
// perft drive ------------------------------

// Search -----------------------------------
// half move counter

int ply;
#define MAX_PLY 64


// killer moves [id][ply]
int killer_moves [2][MAX_PLY];

// history moves [piece][square]
int history_moves[12][64];

/*
      ================================
            Triangular PV table
      --------------------------------
        PV line: e2e4 e7e5 g1f3 b8c6
      ================================
           0    1    2    3    4    5
      
      0    m1   m2   m3   m4   m5   m6
      
      1    0    m2   m3   m4   m5   m6 
      
      2    0    0    m3   m4   m5   m6
      
      3    0    0    0    m4   m5   m6
       
      4    0    0    0    0    m5   m6
      
      5    0    0    0    0    0    m6
*/

// PV length
int pv_length[MAX_PLY];

// PV table
int pv_table[MAX_PLY][MAX_PLY];

// PV search
int follow_pv, score_pv;

// TT ---------------------------------------
#define infinity 50000
#define mate_value 49000
#define mate_score 48000

#define hash_size 800000
#define no_hash_entry 100000

// transposition table hash flags
#define hash_flag_exact  0
#define hash_flag_alpha  1
#define hash_flag_beta   2

// transposition table data structure
typedef struct {
  U64 key;    // hash key of position
  int depth;  // depth to get value
  int flag;   // flag for the type of node (fail-high/fail-low/PV)
  int score;  // score (beta/alpha/PV)
} tt;

tt TT[hash_size];

void clear_tt() {
  for (int index = 0; index < hash_size; index++) {
    // reset TT
    TT[index].key = 0;
    TT[index].depth = 0;
    TT[index].flag = 0;
    TT[index].score = 0;
  }
}

static inline int probe_tt(int alpha, int beta, int depth) {
  tt *hash_entry = &TT[hash_key % hash_size];

  if (hash_entry->key == hash_key) {
    if (hash_entry->depth >= depth) {
      if (hash_entry->flag == hash_flag_exact) {
        int score = hash_entry->score;

        // ajust mate score to distance from root node
        if (score < - mate_score) score += ply;
        if (score > mate_score) score -= ply;

        return score;
      }
      if ((hash_entry->flag == hash_flag_alpha) &&  hash_entry->score <= alpha) {

        return alpha;
      }
      if ((hash_entry->flag == hash_flag_beta) &&  hash_entry->score >= beta) {
        return beta;
      }
    }
  }
  return no_hash_entry;
}

static inline void write_tt_entry(int flag, int score, int depth) {
  // always replace scheme
  tt *hash_entry = &TT[hash_key % hash_size];

  // store mate score independently from distance to root node
  if (score < - mate_score) score -= ply;
  if (score > mate_score) score += ply;

  hash_entry->key = hash_key;
  hash_entry->depth = depth;
  hash_entry->flag = flag;
  hash_entry->score = score;
}
// TT ---------------------------------------

static inline int score_move(int move) {
  // PV move scoring is allowed
  if (score_pv) {
    // dealing with the PV move
    if (pv_table[0][ply] == move) {
      // disable score pv flag
      score_pv = 0;

      // give PV move the highest score to search first
      return 20000;
    }
  }

  // score capture move
  if (get_move_capture_f(move)) {
    // score move by MVV LVA lookup [attackers][victim]

    // init target piece
    int target_piece = P;


    int start_piece = (side == white) ? p : P;
    int end_piece = (side == white) ? k : K;

    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
      if (get_bit(bitboards[bb_piece], get_move_target(move))) {
          target_piece = bb_piece;
          break;
      }
    }

    return mvv_lva[get_move_piece(move)][target_piece] + 10000;
  }

  // score quiet move
  else {
    // score 1st killer move
    if (killer_moves[0][ply] == move)
      return 9000;

    // score 2nd killer move
    else if (killer_moves[1][ply] == move)
      return 8000;

    // score history move
    else
      return history_moves[get_move_piece(move)][get_move_target(move)];
  }

  return 0;
}

static inline void enable_pv_scoring(moves *move_list) {
  // disable following pv
  follow_pv = 0;

  for (int move_count = 0; move_count < move_list->count; move_count++) {
    // make sure we hit PV move
    if (pv_table[0][ply] == move_list->moves[move_count])
    {
      // enable move scoring
      score_pv = 1;

      // enable following pv
      follow_pv = 1;
    }
  }
}

static inline void sort_moves(moves *move_list) {
  // move scores
  int move_scores[move_list->count];

  // score all the moves in the move list
  for (int move_count = 0; move_count < move_list->count; move_count++) {
    move_scores[move_count] = score_move(move_list->moves[move_count]);
  }

  // sort move list
  // loop over current move within a move list
  for (int current_move = 0; current_move < move_list->count; current_move++)
  {
      // loop over next move within a move list
      for (int next_move = current_move + 1; next_move < move_list->count; next_move++)
      {
          // compare current and next move scores
          if (move_scores[current_move] < move_scores[next_move])
          {
              // swap scores
              int temp_score = move_scores[current_move];
              move_scores[current_move] = move_scores[next_move];
              move_scores[next_move] = temp_score;
              
              // swap moves
              int temp_move = move_list->moves[current_move];
              move_list->moves[current_move] = move_list->moves[next_move];
              move_list->moves[next_move] = temp_move;
          }
      }
  }

}

static inline int repetition_detection() {
  for (int index = 0; index < repetition_index; index++) {
    if (repetition_table[index] == hash_key)
      return 1;
  }
  return 0;
}

static inline int quiescence(int alpha, int beta) {
  // every 2047 nodes
  if((nodes & 2047 ) == 0)
    // "listen" to the GUI/user input
    communicate();

  // increment nodes count
  nodes++;

  // evaluate position
  int evaluation = evaluate();

  // fail-hard beta cutoff
  if (evaluation >= beta) {
    return beta;
  }

  if (ply > MAX_PLY - 1)
    return evaluation;

  // found a better move
  if (evaluation > alpha) {
    // PV node
    alpha = evaluation;
  }


  // generate moves
  moves move_list[1];
  generate_moves(move_list);

  // sort moves
  sort_moves(move_list);

  // loop over generated moves
  for (int move_count = 0; move_count < move_list->count; move_count++) {
    //  preserve board state
    copy_board();

    // increment ply
    ply++;
    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if (!make_move(move_list->moves[move_count], only_captures))  {
      // decrement ply
      ply--;
      repetition_index--;

      //take move back
      continue;
    }
    // increment legal moves
  
    // score current move
    int score = -quiescence(-beta, -alpha);

    // decrement ply
    ply--;
    repetition_index--;
    //take move back
    take_back();

    // return 0 if time is up
    if(stopped == 1) return 0;

    // fail-hard beta cutoff
    if (score >= beta) {
      return beta;
    }
    // found a better move
    if (score > alpha) {
      // PV node
      alpha = score;
      
    }

  }
  return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

static inline int lmr_condition(int move, int moves_searched, int in_check, int depth) {
  return  (moves_searched >= full_depth_moves) &&
          (depth >= reduction_limit) &&
          (in_check == 0) && 
          (get_move_capture_f(move) == 0) &&
          (get_move_promoted(move) == 0);
}

// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth, int null_pruning) {
  // every 2047 nodes
  if((nodes & 2047 ) == 0)
    // "listen" to the GUI/user input
    communicate();

  if (ply && repetition_detection())
    return 0;

  int pv_node = (beta - alpha) > 1;
  int score = probe_tt(alpha, beta, depth);

  if (ply && (score != no_hash_entry) && !pv_node) {
    return score;
  }

  // init PV length
  pv_length[ply] = ply;

  // is king in check
  int in_check = is_square_attacked((side == white) ? get_lsb_index(bitboards[K]) :
                                                      get_lsb_index(bitboards[k]),
                                                      side ^ 1);

  // increase search depth if king has been exposed to check
  if (in_check)
    depth++;

  // recurrsion espace condition
  if (depth == 0)
    // run quiescence
    return quiescence(alpha, beta);

  // static evaluation
  if (ply > MAX_PLY - 1)
    return evaluate();

  // increment nodes count
  nodes++;


  // legal moves counter
  int legal_moves = 0;

  // null move pruning
  if (null_pruning && (!pv_node && depth >= 3 && !in_check && ply)) {

    // make a null move (switch side to play)
    copy_board();
    side ^= 1;
    hash_key ^= side_key;

    if (enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];
    enpassant = no_sq;
    ply++;

    repetition_index++;

    repetition_table[repetition_index] = hash_key;

    // reduction factor
    int r = 2;

    // disable null pruning for next node
    score = -negamax(-beta, - beta + 1, depth - 1 - r, 0);

    // restore board state
    ply--;
    repetition_index--;
    take_back();

    // return 0 if time is up
    if(stopped == 1) return 0;

    if (score >= beta) {
      return beta;
    }
  }

  // generate moves
  moves move_list[1];
  generate_moves(move_list);

  // if we are now following PV lines
  if (follow_pv)
    enable_pv_scoring(move_list);

  // sort moves
  sort_moves(move_list);

  // number of moves already searched
  int moves_searched = 0;

  // loop over generated moves
  for (int move_count = 0; move_count < move_list->count; move_count++) {
    //  preserve board state
    copy_board();

    // increment ply
    ply++;
    repetition_index++;
    repetition_table[repetition_index] = hash_key;

    if (!make_move(move_list->moves[move_count], all_moves))  {
      // decrement ply
      ply--;
      repetition_index--;
      continue;
    }

    // increment legal moves
    legal_moves++;

    // init score
    int score;
    // full depth search
    if (moves_searched == 0)
      // do normal alpha beta search
      score = -negamax(-beta, -alpha, depth - 1, 1);

    // late move reduction (LMR)
    else
    {
      // condition to consider LMR
      if(lmr_condition(move_list->moves[move_count], moves_searched, in_check, depth)) {
        // search current move with reduced depth:
        score = -negamax(-alpha - 1, -alpha, depth - 2, 1);
      }
      else {
        score = alpha + 1;
      }
      
      // principle variation search PVS
      if(score > alpha)
      {
        score = -negamax(-alpha - 1, -alpha, depth-1, 1);
    
        if((score > alpha) && (score < beta))
          score = -negamax(-beta, -alpha, depth-1, 1);
      }
    }

    // decrement ply
    ply--;
    repetition_index--;
    //take move back
    take_back();

    // return 0 if time is up
    if(stopped == 1) return 0;

    // increment the counter of moves searched so far
    moves_searched++;

    // found a better move
    if (score > alpha) {
      write_tt_entry(hash_flag_exact, score, depth);

      // store history moves
      if (!get_move_capture_f(move_list->moves[move_count]))
        history_moves[get_move_piece(move_list->moves[move_count])][get_move_target(move_list->moves[move_count])] += depth;

      // PV node
      alpha = score;

      // write PV move
      pv_table[ply][ply] = move_list->moves[move_count];
            
      // loop over the next ply
      for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
        // copy move from deeper ply into a current ply's line
        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
      
      // adjust PV length
      pv_length[ply] = pv_length[ply + 1];

      // fail-hard beta cutoff
      if (score >= beta) {
        write_tt_entry(hash_flag_beta, beta, depth);

        // store killer moves
        if (!get_move_capture_f(move_list->moves[move_count])) {
          killer_moves[1][ply] = killer_moves[0][ply];
          killer_moves[0][ply] = move_list->moves[move_count];
        }
        return beta;
      }

    }
  }

  if (legal_moves == 0) {
    // return mating score
    if (in_check) {
      return -mate_value + ply; // number of move to get to mate (fav. faster mate)
    }

    // stalemate
    else
      return 0;
  }
  write_tt_entry(hash_flag_alpha, alpha, depth);
  return alpha;
}

// file masks [square]
U64 file_masks[64];

// rank masks [square]
U64 rank_masks[64];

// isolated pawn masks [square]
U64 isolated_masks[64];

// white passed pawn masks [square]
U64 white_passed_masks[64];

// black passed pawn masks [square]
U64 black_passed_masks[64];

// extract rank from a square [square]
const int get_rank[64] =
{
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

// double pawns penalty
const int double_pawn_penalty = -10;

// isolated pawn penalty
const int isolated_pawn_penalty = -10;

// passed pawn bonus
const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 }; 

// semi open file score
const int semi_open_file_score = 10;

// open file score
const int open_file_score = 15;

// king's shield bonus
const int king_shield_bonus = 5;

// set file or rank mask
U64 set_file_rank_mask(int file_number, int rank_number)
{
    // file or rank mask
    U64 mask = 0ULL;
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            if (file_number != -1)
            {
                // on file match
                if (file == file_number)
                    // set bit on mask
                    mask |= set_bit(mask, square);
            }
            
            else if (rank_number != -1)
            {
                // on rank match
                if (rank == rank_number)
                    // set bit on mask
                    mask |= set_bit(mask, square);
            }
        }
    }
    
    // return mask
    return mask;
}

// init evaluation masks
void init_evaluation_masks()
{
    /******** Init file masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            file_masks[square] |= set_file_rank_mask(file, -1);
        }
    }
    
    /******** Init rank masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            rank_masks[square] |= set_file_rank_mask(-1, rank);
        }
    }
    
    /******** Init isolated masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
            isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
        }
    }
    
    /******** White passed masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
            white_passed_masks[square] |= set_file_rank_mask(file, -1);
            white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);
            
            // loop over redudant ranks
            for (int i = 0; i < (8 - rank); i++)
                // reset redudant bits 
                white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
        }
    }
    
    /******** Black passed masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
            black_passed_masks[square] |= set_file_rank_mask(file, -1);
            black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);
            
            // loop over redudant ranks
            for (int i = 0; i < rank + 1; i++)
                // reset redudant bits 
                black_passed_masks[square] &= ~rank_masks[i * 8 + file];            
        }
    }
}

static inline int evaluate() {
  // static evaluation score
  int score = 0;
  
  // current pieces bitboard copy
  U64 bitboard;
  
  // init piece & square
  int piece, square;
  
  // penalties
  int double_pawns = 0;
  
  // loop over piece bitboards
  for (int bb_piece = P; bb_piece <= k; bb_piece++)
  {
    // init piece bitboard copy
    bitboard = bitboards[bb_piece];
    
    // loop over pieces within a bitboard
    while (bitboard)
    {
      // init piece
      piece = bb_piece;
      
      // init square
      square = get_lsb_index(bitboard);
      
      // score material weights
      score += material_score[piece];
      
      // score positional piece scores
      switch (piece)
      {
        // evaluate white pieces
        case P:
          // positional score
          score += pawn_score[square];
          
          // double pawn penalty
          double_pawns = count_bits(bitboards[P] & file_masks[square]);
          
          // on double pawns (tripple, etc)
          if (double_pawns > 1)
            score += double_pawns * double_pawn_penalty;
          
          // on isolated pawn
          if ((bitboards[P] & isolated_masks[square]) == 0)
              // give an isolated pawn penalty
            score += isolated_pawn_penalty;
          
          // on passed pawn
          if ((white_passed_masks[square] & bitboards[p]) == 0)
            // give passed pawn bonus
            score += passed_pawn_bonus[get_rank[square]];

          break;

        case N: score += knight_score[square]; break;
        case B:
          // positional scores
          score += bishop_score[square];
          
          // mobility
          score += count_bits(get_bishop_attacks(square, occupancies[both]));
          
          break;
        
        case R:
          // positional score
          score += rook_score[square];
          
          // semi open file
          if ((bitboards[P] & file_masks[square]) == 0)
              // add semi open file bonus
              score += semi_open_file_score;
          
          // semi open file
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
              // add semi open file bonus
              score += open_file_score;
          
          break;
        
        case Q:
          // mobility
          score += count_bits(get_queen_attacks(square, occupancies[both]));
          break;
        
        case K:
          // posirional score
          score += king_score[square];
          
          // semi open file
          if ((bitboards[P] & file_masks[square]) == 0)
              // add semi open file penalty
              score -= semi_open_file_score;
          
          // semi open file
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
              // add semi open file penalty
              score -= open_file_score;
          
          // king safety bonus
          score += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
          break;

        // evaluate black pieces
        case p:
          // positional score
          score -= pawn_score[mirror_score[square]];

          // double pawn penalty
          double_pawns = count_bits(bitboards[p] & file_masks[square]);
          
          // on double pawns (tripple, etc)
          if (double_pawns > 1)
              score -= double_pawns * double_pawn_penalty;
          
          // on isolated pawnd
          if ((bitboards[p] & isolated_masks[square]) == 0)
              // give an isolated pawn penalty
              score -= isolated_pawn_penalty;
          
          // on passed pawn
          if ((black_passed_masks[square] & bitboards[P]) == 0)
              // give passed pawn bonus
              score -= passed_pawn_bonus[get_rank[mirror_score[square]]];

          break;

        case n: score -= knight_score[mirror_score[square]]; break;
        
        case b:
          // positional score
          score -= bishop_score[mirror_score[square]];
          
          // mobility
          score -= count_bits(get_bishop_attacks(square, occupancies[both]));
          break;
        
        case r:
          // positional score
          score -= rook_score[mirror_score[square]];
          
          // semi open file
          if ((bitboards[p] & file_masks[square]) == 0)
              // add semi open file bonus
              score -= semi_open_file_score;
          
          // semi open file
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
              // add semi open file bonus
              score -= open_file_score;
          
          break;
        
        case q:
          // mobility
          score -= count_bits(get_queen_attacks(square, occupancies[both]));
          break;
        
        case k:
          // positional score
          score -= king_score[mirror_score[square]];
          
          // semi open file
          if ((bitboards[p] & file_masks[square]) == 0)
              // add semi open file penalty
              score += semi_open_file_score;
          
          // semi open file
          if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
              // add semi open file penalty
              score += open_file_score;
          
          // king safety bonus
          score -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
          break;
      }

      // pop ls1b
      pop_bit(bitboard, square);
    }
  }
    
  // return final evaluation based on side
  return (side == white) ? score : -score;
}


void search_position(int depth) {
  // define score variable
  int score = 0;

  // reset nodes counter
  nodes = 0;

  // reset "time is up" flag
  stopped = 0;

  // reset follow pv flags
  follow_pv = 0;
  score_pv = 0;

  // clear helper data structures for search
  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_moves, 0, sizeof(history_moves));
  memset(pv_length, 0, sizeof(pv_length));
  memset(pv_table, 0, sizeof(pv_table));

  // init alpha beta
  int alpha = -infinity, beta = infinity;

  // iterative deepening
  for (int current_depth = 1; current_depth <= depth; current_depth++) {
    // if time is up
    if(stopped == 1)
			// stop calculating and return best move so far 
			break;

    // enable follow pv flag
    follow_pv = 1;

    // find best move
    score = negamax(alpha, beta, current_depth, 0);

    // window fail go for full window
    if ((score <= alpha) || (score >= beta)) {
      alpha = -infinity;
      beta = infinity;

      printf("windows fail rerun at full length\n");
      score = negamax(alpha, beta, current_depth, 0);
    }

    if(stopped == 1)
			// stop calculating and return best move so far 
			break;

    // aspiration window
    alpha = score - 100;
    beta = score + 100;
    
    if (score > -mate_value && score < -mate_score) {
      printf("info score mate %d depth %d nodes %ld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - starttime);
    }
    else if (score > mate_score && score < mate_value) {
      printf("info score mate %d depth %d nodes %ld time %d pv ", (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - starttime); 
    }  
    else
      printf("info score cp %d depth %d nodes %ld time %d pv ", score, current_depth, nodes, get_time_ms() - starttime);
    
    // loop over the moves within a PV line
    for (int count = 0; count < pv_length[0]; count++)
    {
      // print PV move
      print_move(pv_table[0][count]);
      printf(" ");
    }
    // print new line
    printf("\n");

    if ((score > -mate_value && score < -mate_score) || (score > mate_score && score < mate_value))
      break;
  }

  printf("bestmove ");
  print_move(pv_table[0][0]);
  printf("\n");

}

// Main -------------------------------------
int main() {
  // init all
  init_all();
  clear_tt();

  // debug mode variable
  int debug = 0;
  #if DEBUG
  debug = 1;
  #endif

  if (debug) {
    printf("debug mode\n");

    parse_fen(tricky_position);
    print_board();
    
    int start  = get_time_ms();
    search_position(13);
    printf("Time elapsed: %dms\n\n", get_time_ms() - start);
  }
  else
    // connect to the GUI
    uci_loop();
  
  return 0;
}
// Main -------------------------------------
