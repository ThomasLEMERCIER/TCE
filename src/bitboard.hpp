#pragma once

#include "definition.hpp"

using Bitboard = U64;

// bishop:rook maximal occupancy subset size
constexpr int bishop_magic_size = 512;
constexpr int rook_magic_size = 4096;
constexpr int max_magic_size = 4096;

constexpr Bitboard FILE_A_BB = 0x101010101010101ULL;
constexpr Bitboard FILE_B_BB = FILE_A_BB << 1;
constexpr Bitboard FILE_C_BB = FILE_A_BB << 2;
constexpr Bitboard FILE_D_BB = FILE_A_BB << 3;
constexpr Bitboard FILE_E_BB = FILE_A_BB << 4;
constexpr Bitboard FILE_F_BB = FILE_A_BB << 5;
constexpr Bitboard FILE_G_BB = FILE_A_BB << 6;
constexpr Bitboard FILE_H_BB = FILE_A_BB << 7;

constexpr Bitboard RANK_1_BB = 0xff00000000000000ULL;
constexpr Bitboard RANK_2_BB = RANK_1_BB >> (8 * 1);
constexpr Bitboard RANK_3_BB = RANK_1_BB >> (8 * 2);
constexpr Bitboard RANK_4_BB = RANK_1_BB >> (8 * 3);
constexpr Bitboard RANK_5_BB = RANK_1_BB >> (8 * 4);
constexpr Bitboard RANK_6_BB = RANK_1_BB >> (8 * 5);
constexpr Bitboard RANK_7_BB = RANK_1_BB >> (8 * 6);
constexpr Bitboard RANK_8_BB = RANK_1_BB >> (8 * 7);

constexpr Bitboard NOT_A_FILE_BB = ~FILE_A_BB;
constexpr Bitboard NOT_H_FILE_BB = ~FILE_H_BB;
constexpr Bitboard NOT_AB_FILE_BB = ~(FILE_A_BB | FILE_B_BB);
constexpr Bitboard NOT_GH_FILE_BB = ~(FILE_G_BB | FILE_H_BB);

constexpr Bitboard SquareBB[SQUARE_NB] = {
  (1ULL << A8), (1ULL << B8), (1ULL << C8), (1ULL << D8), (1ULL << E8), (1ULL << F8), (1ULL << G8), (1ULL << H8),
  (1ULL << A7), (1ULL << B7), (1ULL << C7), (1ULL << D7), (1ULL << E7), (1ULL << F7), (1ULL << G7), (1ULL << H7),
  (1ULL << A6), (1ULL << B6), (1ULL << C6), (1ULL << D6), (1ULL << E6), (1ULL << F6), (1ULL << G6), (1ULL << H6),
  (1ULL << A5), (1ULL << B5), (1ULL << C5), (1ULL << D5), (1ULL << E5), (1ULL << F5), (1ULL << G5), (1ULL << H5),
  (1ULL << A4), (1ULL << B4), (1ULL << C4), (1ULL << D4), (1ULL << E4), (1ULL << F4), (1ULL << G4), (1ULL << H4),
  (1ULL << A3), (1ULL << B3), (1ULL << C3), (1ULL << D3), (1ULL << E3), (1ULL << F3), (1ULL << G3), (1ULL << H3),
  (1ULL << A2), (1ULL << B2), (1ULL << C2), (1ULL << D2), (1ULL << E2), (1ULL << F2), (1ULL << G2), (1ULL << H2),
  (1ULL << A1), (1ULL << B1), (1ULL << C1), (1ULL << D1), (1ULL << E1), (1ULL << F1), (1ULL << G1), (1ULL << H1),
};

extern Bitboard pawn_attacks[COLOR_NB][SQUARE_NB];
extern Bitboard knight_attacks[SQUARE_NB];
extern Bitboard king_attacks[SQUARE_NB];
extern Bitboard bishop_masks[SQUARE_NB];
extern Bitboard rook_masks[SQUARE_NB];
extern Bitboard bishop_attacks[SQUARE_NB][bishop_magic_size];
extern Bitboard rook_attacks[SQUARE_NB][rook_magic_size];

void init_leapers_attacks();
void init_slider_attacks(Sliding_Piece piece);

void init_attacks();

Bitboard mask_pawn_attacks(Color side, Square square);
Bitboard mask_knight_attacks(Square square);
Bitboard mask_king_attacks(Square square);

Bitboard get_occupancy_subset(int index, int bits_in_mask, Bitboard attack_mask);
Bitboard mask_bishop_attacks(Square square);
Bitboard mask_rook_attacks(Square square);
Bitboard bishop_attacks_on_the_fly(Square square, Bitboard occupancy);
Bitboard rook_attacks_on_the_fly(Square square, Bitboard occupancy);

Bitboard find_magic_number(Square square, int relevant_bits, Sliding_Piece piece);
void init_magic_numbers();

Bitboard get_pawn_attacks(Square square, Color side);
Bitboard get_knight_attacks(Square square);
Bitboard get_bishop_attacks(Square square, Bitboard occupancy);
Bitboard get_rook_attacks(Square square, Bitboard occupancy);
Bitboard get_queen_attacks(Square square, Bitboard occupancy);
Bitboard get_king_attacks(Square square);

// relevant occupancy bit count for every square on board
constexpr int bishop_relevant_bits[SQUARE_NB] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};
constexpr int rook_relevant_bits[SQUARE_NB] = {
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
constexpr U64 rook_magic_numbers[SQUARE_NB] = {
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
constexpr U64 bishop_magic_numbers[SQUARE_NB] = {
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

void print_bitboard(Bitboard bitboard);

int count_bits(Bitboard bitboard);
void set_bit(Bitboard& bitboard, Square square);
Bitboard get_bit(Bitboard bitboard, Square square);
void pop_bit(Bitboard& bitboard, Square square);
Square get_lsb_index(Bitboard bitboard);

template <Direction d>
Bitboard shift(Bitboard bitboard) { return (d == Direction::LEFT) ? ((bitboard >> 1) & ~FILE_H_BB): (d == Direction::RIGHT) ? ((bitboard << 1) & ~FILE_A_BB) : 
                                           (d == Direction::UP)   ? (bitboard >> 8): (d == Direction::DOWN) ? (bitboard << 8) : bitboard; };

Bitboard get_file_bb(File file);
Bitboard get_rank_bb(Rank rank);
Rank get_rank(Square square);
File get_file(Square square);

template <Piece piece>
Bitboard get_attacks_bb(Square square, Bitboard occupancy) {
  return (piece == Piece::WN) || (piece == Piece::BN) ? get_knight_attacks(square)            :
         (piece == Piece::WB) || (piece == Piece::BB) ? get_bishop_attacks(square, occupancy) :
         (piece == Piece::WR) || (piece == Piece::BR) ? get_rook_attacks(square, occupancy)   :
         (piece == Piece::WQ) || (piece == Piece::BQ) ? get_queen_attacks(square, occupancy)  :
         (piece == Piece::WK) || (piece == Piece::BK) ? get_king_attacks(square)              : 0ULL;
}
