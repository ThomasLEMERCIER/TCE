#include <stdio.h>
#include <string.h>

#include "bitboard.hpp"
#include "rng.hpp"

Bitboard pawn_attacks[COLOR_NB][SQUARE_NB];
Bitboard knight_attacks[SQUARE_NB];
Bitboard king_attacks[SQUARE_NB];
Bitboard bishop_masks[SQUARE_NB];
Bitboard rook_masks[SQUARE_NB];
Bitboard bishop_attacks[SQUARE_NB][bishop_magic_size];
Bitboard rook_attacks[SQUARE_NB][rook_magic_size];

U64 mask_pawn_attacks(Color side, Square square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if (side == Color::white) {
    if ((bitboard >> 7) & NOT_A_FILE_BB) attacks |= (bitboard >> 7);
    if ((bitboard >> 9) & NOT_H_FILE_BB) attacks |= (bitboard >> 9);
  }
  else {
    if ((bitboard << 7) & NOT_H_FILE_BB) attacks |= (bitboard << 7);
    if ((bitboard << 9) & NOT_A_FILE_BB) attacks |= (bitboard << 9);
  }

  return attacks;
}

U64 mask_knight_attacks(Square square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  if ((bitboard >> 17) & NOT_H_FILE_BB) attacks |= (bitboard >> 17);
  if ((bitboard >> 15) & NOT_A_FILE_BB) attacks |= (bitboard >> 15);
  if ((bitboard >> 10) & NOT_GH_FILE_BB) attacks |= (bitboard >> 10);
  if ((bitboard >> 6) & NOT_AB_FILE_BB) attacks |= (bitboard >> 6);

  if ((bitboard << 17) & NOT_A_FILE_BB) attacks |= (bitboard << 17);
  if ((bitboard << 15) & NOT_H_FILE_BB) attacks |= (bitboard << 15);
  if ((bitboard << 10) & NOT_AB_FILE_BB) attacks |= (bitboard << 10);
  if ((bitboard << 6) & NOT_GH_FILE_BB) attacks |= (bitboard << 6);

  return attacks;
}

Bitboard mask_king_attacks(Square square) {
  Bitboard attacks = 0ULL;
  Bitboard bitboard = 0ULL;
  set_bit(bitboard, square);

  if ((bitboard >> 9) & NOT_H_FILE_BB)   attacks |= (bitboard >> 9);
  if (bitboard >> 8)                  attacks |= (bitboard >> 8);
  if ((bitboard >> 7) & NOT_A_FILE_BB)   attacks |= (bitboard >> 7);
  if ((bitboard >> 1) & NOT_H_FILE_BB)   attacks |= (bitboard >> 1);

  if ((bitboard << 9) & NOT_A_FILE_BB)   attacks |= (bitboard << 9);
  if (bitboard << 8)                  attacks |= (bitboard << 8);
  if ((bitboard << 7) & NOT_H_FILE_BB)   attacks |= (bitboard << 7);
  if ((bitboard << 1) & NOT_A_FILE_BB)   attacks |= (bitboard << 1);

  return attacks;
}

void init_leapers_attacks() {
  for (Square square = Square::FIRST_SQ; square < Square::SQUARE_NB; ++square) {
    pawn_attacks[Color::white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[Color::black][square] = mask_pawn_attacks(black, square);

    knight_attacks[square] = mask_knight_attacks(square);

    king_attacks[square] = mask_king_attacks(square);
  }
}

void init_slider_attacks(Sliding_Piece piece) {
  for (Square square = Square::FIRST_SQ; square < Square::SQUARE_NB; ++square) {
    Bitboard attack_mask;
    int relevant_bits_count;

    if (piece == Sliding_Piece::bishop) {
      attack_mask = mask_bishop_attacks(square);
      bishop_masks[square] = attack_mask;
      relevant_bits_count = bishop_relevant_bits[square];
    }
    else {
      attack_mask = mask_rook_attacks(square);
      rook_masks[square] = attack_mask;
      relevant_bits_count = rook_relevant_bits[square];
    }

    int occupancy_subset_size = (1 << relevant_bits_count);
    Bitboard occupancy;

    // loop over all subset of occupancy to setup the exact attacks and occupancies
    for (int index = 0; index < occupancy_subset_size; index++) {
      if (piece == Sliding_Piece::bishop) {
        occupancy = get_occupancy_subset(index, relevant_bits_count, attack_mask);
        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);

        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
      }
      else {
        occupancy = get_occupancy_subset(index, relevant_bits_count, attack_mask);
        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);

        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
      }
    }
  }
}

Bitboard mask_bishop_attacks(Square square) {
  Bitboard attacks = 0ULL;

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++) attacks |= (1ULL << (r * 8 + f));
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--) attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--) attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++) attacks |= (1ULL << (r * 8 + f));

  return attacks;
}

Bitboard mask_rook_attacks(Square square) {
  Bitboard attacks = 0ULL;

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1; r < 7; r++) attacks |= (1ULL << (r * 8 + tf));
  for (r = tr - 1; r > 0; r--) attacks |= (1ULL << (r * 8 + tf));
  for (f = tf - 1; f > 0; f--) attacks |= (1ULL << (tr * 8 + f));
  for (f = tf + 1; f < 7; f++) attacks |= (1ULL << (tr * 8 + f));

  return attacks;
}

Bitboard bishop_attacks_on_the_fly(Square square, Bitboard occupancy) {
  Bitboard attacks = 0ULL;

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

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

Bitboard rook_attacks_on_the_fly(Square square, Bitboard occupancy) {
  Bitboard attacks = 0ULL;

  int r, f;
  int tr = square / 8;
  int tf = square % 8;

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

Bitboard get_occupancy_subset(int index, int bits_in_mask, U64 attack_mask) {
  // occupancy mask
  Bitboard occupancy = 0ULL;

  // loop over all posible bit in mask
  for (int count = 0; count < bits_in_mask; count++) {
    // get LSB index of attack mask
    Square square = get_lsb_index(attack_mask);

    // pop LSB in attack mask
    pop_bit(attack_mask, square);

    // set bit in occupancy if bit set in index
    if (index & (1 << count))
      occupancy |= (1ULL << square);
  }
  return occupancy;
}

U64 find_magic_number(Square square, int relevant_bits, Sliding_Piece piece) {
  Bitboard occupancies[max_magic_size];
  Bitboard attacks[max_magic_size];
  Bitboard used_attacks[max_magic_size];
  Bitboard attack_mask;

  if (piece == Sliding_Piece::bishop)
    attack_mask = mask_bishop_attacks(square);
  else
    attack_mask = mask_rook_attacks(square);

  int occupancy_subset_size = 1 << relevant_bits;

  int index;
  // loop over all subset of occupancy to setup the exact attacks and occupancies
  for (index = 0; index < occupancy_subset_size; index++) {
    occupancies[index] = get_occupancy_subset(index, relevant_bits, attack_mask);

    if (piece == Sliding_Piece::bishop)
      attacks[index] = bishop_attacks_on_the_fly(square, occupancies[index]);
    else 
      attacks[index] = rook_attacks_on_the_fly(square, occupancies[index]);
  }

  RNG rng((unsigned int)1804289383);
  U64 magic_number;

  // test magic numbers loop
  for (int random_count = 0; random_count < 100000000; random_count++) {
    magic_number = rng.magic_rand();

    if (count_bits((Bitboard)((attack_mask * magic_number) & 0xFF00000000000000)) < 6) continue;

    memset(used_attacks, 0ULL, sizeof(used_attacks));
    int fail;

    // test magic number : loop over all subset of occupanc
    for (index = 0, fail = 0; !fail && index < occupancy_subset_size; index++) {
      int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));

      // if magic index unused
      if (used_attacks[magic_index] == 0ULL)
        used_attacks[magic_index] = attacks[index];
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
  for (Square square = Square::FIRST_SQ; square < Square::SQUARE_NB; ++square)
    // print rook magic numbers
    printf(" 0x%llxULL,\n", find_magic_number(square, rook_relevant_bits[square], rook));

  // loop over 64 board squares
  for (Square square = Square::FIRST_SQ; square < Square::SQUARE_NB; ++square)
    // print bishop magic numbers
    printf(" 0x%llxULL,\n", find_magic_number(square, bishop_relevant_bits[square], bishop));
}

void init_attacks() {
  init_leapers_attacks();
  init_slider_attacks(Sliding_Piece::bishop);
  init_slider_attacks(Sliding_Piece::rook);
}

Bitboard get_pawn_attacks(Square square, Color side) {
  return pawn_attacks[side][square];
}

Bitboard get_knight_attacks(Square square) {
  return knight_attacks[square];
}

Bitboard get_bishop_attacks(Square square, Bitboard occupancy) {
  occupancy &= bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= 64 - bishop_relevant_bits[square];
  return bishop_attacks[square][occupancy];
}

Bitboard get_rook_attacks(Square square, Bitboard occupancy) {
  occupancy &= rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= 64 - rook_relevant_bits[square];
  return rook_attacks[square][occupancy];
}

Bitboard get_queen_attacks(Square square, Bitboard occupancy) {
  Bitboard bishop_occupancy = occupancy;
  Bitboard rook_occupancy = occupancy;

  rook_occupancy &= rook_masks[square];
  rook_occupancy *= rook_magic_numbers[square];
  rook_occupancy >>= 64 - rook_relevant_bits[square];

  bishop_occupancy &= bishop_masks[square];
  bishop_occupancy *= bishop_magic_numbers[square];
  bishop_occupancy >>= 64 - bishop_relevant_bits[square];

  return rook_attacks[square][rook_occupancy] | bishop_attacks[square][bishop_occupancy];
}

Bitboard get_king_attacks(Square square) {
  return king_attacks[square];
}

int count_bits(Bitboard bitboard) {
  int count = 0;

  // consecutively reset LSB
  while (bitboard) {
    count++;
    // reset LSB
    bitboard &= bitboard -1;
  }
  return count;
}

void set_bit(Bitboard& bitboard, Square square) {
  bitboard |= SquareBB[square];
}

Bitboard get_bit(Bitboard bitboard, Square square) {
  return bitboard & SquareBB[square];
} 

void pop_bit(Bitboard& bitboard, Square square) {
  if (get_bit(bitboard, square))
    bitboard ^= SquareBB[square];
}

Square get_lsb_index(Bitboard bitboard) {
  assert(b);
  return Square(__builtin_ctzll(bitboard));
}

Bitboard get_file_bb(File file) {
  return (FILE_A_BB << file);
}

Bitboard get_rank_bb(Rank rank) {
  return (RANK_1_BB >> (8 * (7 - rank)));
}

Rank get_rank(Square square) { return Rank((int)square >> 3); }
File get_file(Square square) { return File((int)square & 7); }

void print_bitboard(Bitboard bitboard) { 
 for (Rank rank = Rank::RANK_8; rank <= RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= FILE_H; ++file) {
      Square square = get_square(rank, file);

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
