#include "position.hpp"
#include <stdio.h>

U64 piece_keys[12][64];
U64 side_key;
U64 enpassant_keys[64];
U64 castle_keys[64];

Position::Position(Position* pos) {
  memcpy(bitboards, pos->bitboards, sizeof(bitboards));
  memcpy(occupancies, pos->occupancies, sizeof(occupancies));
  memcpy(repetition_table, repetition_table, sizeof(repetition_table));
  
  side = pos->side;
  enpassant = pos->enpassant;
  castle_rights = pos->castle_rights;

  hash_key = pos->hash_key;
  repetition_index = pos->repetition_index;
  ply = pos->ply;
};

void Position::set(const char* fenStr) {
  memset(this, 0, sizeof(Position));

  for (Rank rank = Rank::RANK_8; rank <= RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= FILE_H; ++file) {
      Square square = get_square(rank, file);

      // matching piece
      if ((*fenStr >= 'a' && *fenStr <= 'z') || (*fenStr >= 'A' && *fenStr <= 'Z')) {
        Piece piece = char_pieces.at(*fenStr);
        set_bit(bitboards[piece], square);
        fenStr++;
      }
      // matching empty squares
      else if ((*fenStr >= '0' && *fenStr <= '9')) {
        int offset = *fenStr - '0' - 1;
        file = File((int)file + offset);
        fenStr++;
      }
      // match rank sep
      if (*fenStr == '/') {
        fenStr++;
      }
    }
  }

  // skip whitespace to parse side to move
  fenStr++;
  (*fenStr == 'w') ? (side = Color::white) : (side = Color::black);

  // skip color and whitespace to parse castling right
  fenStr += 2;
  while (*fenStr != ' ') {
    switch (*fenStr)
    {
      case 'K': castle_rights |= Castle_Right::wk; break;
      case 'Q': castle_rights |= Castle_Right::wq; break;
      case 'k': castle_rights |= Castle_Right::bk; break;
      case 'q': castle_rights |= Castle_Right::bq; break;
      default: break;
    }
    fenStr++;
  }

  // skip whitespace to parse en passant square 
  fenStr++;
  if (*fenStr == '-') {
    enpassant = Square::no_sq;
  }
  else {
    int file = fenStr[0] - 'a';
    int rank = 8 - (fenStr[1] - '0');
    enpassant = get_square(Rank(rank), File(file));
  }

  // loop over white pieces bitboards
  for (Piece piece = Piece::P; piece <= Piece::K; ++piece)
    occupancies[Color::white] |= bitboards[piece];
  // loop over black pieces bitboards
  for (Piece piece = Piece::p; piece <= Piece::k; ++piece)
    occupancies[Color::black] |= bitboards[piece];

  occupancies[Color::both] |= occupancies[Color::white];
  occupancies[Color::both] |= occupancies[Color::black];

  hash_key = generate_hash_key(this);

  repetition_index = 0;
  repetition_table[repetition_index] = hash_key;
}

U64 generate_hash_key(Position* pos) {
  U64 final_key = 0ULL;
  Bitboard bitboard;
  
  // loop over piece bitboards
  for (Piece piece = Piece::P; piece <= Piece::k; ++piece) {
    bitboard = pos->bitboards[piece];
    while (bitboard) {
      Square square = get_lsb_index(bitboard);
      
      final_key ^= piece_keys[piece][square];

      pop_bit(bitboard, square);
    }
  }
  
  // if enpassant square is on board
  if (pos->enpassant != Square::no_sq)
    final_key ^= enpassant_keys[pos->enpassant];
  
  // castling hash
  final_key ^= castle_keys[pos->castle_rights];
  
  // hash the side only if black is to move
  if (pos->side == Color::black) final_key ^= side_key;

  return final_key;
};

void init_random_keys() {
  RNG rng((unsigned long)1804289383);

  // loop over pieces and squares
  for (Piece piece = Piece::P; piece <= Piece::k; ++piece) {
    for (Square square = Square::FIRST_SQ; square < Square::SQUARE_NB; ++square)
      // init random piece keys
      piece_keys[piece][square] = rng.rand64();
  }
  
  // loop over squares
  for (Square square = Square::FIRST_SQ; square < Square::SQUARE_NB; ++square)
    // init random enpassant keys
    enpassant_keys[square] = rng.rand64();
  
  // loop over all castling variations
  for (int index = 0; index < 16; index++)
    // init castling keys
    castle_keys[index] = rng.rand64();
      
  // init random side key
  side_key = rng.rand64();
}

int is_square_attacked(Position* pos, Square square, Color side) {
  // using symetry of attack pattern

  // white pawn attack
  if ((side == Color::white) && (pawn_attacks[Color::black][square] & pos->bitboards[P])) return 1;

  // black pawn attack
  if ((side == Color::black) && (pawn_attacks[Color::white][square] & pos->bitboards[p])) return 1;

  // knight attack
  if (knight_attacks[square] & ((side == Color::white) ? pos->bitboards[N] :pos-> bitboards[n])) return 1;

  // bishop attack
  if (get_bishop_attacks(square, pos->occupancies[Color::both]) & ((side == Color::white) ? pos->bitboards[B] : pos->bitboards[b])) return 1;

  // bishop attack
  if (get_rook_attacks(square, pos->occupancies[Color::both]) & ((side == Color::white) ? pos->bitboards[R] : pos->bitboards[r])) return 1;

  // queen attack
  if (get_queen_attacks(square, pos->occupancies[Color::both]) & ((side == Color::white) ? pos->bitboards[Q] : pos->bitboards[q])) return 1;

  // king attack
  if (king_attacks[square] & ((side == Color::white) ? pos->bitboards[K] : pos->bitboards[k])) return 1;

  return 0;
}


int make_move(Position* pos, Move move, Move_Type move_flag) {
  if (move_flag == Move_Type::all_moves) {
    // parse move
    Square source_square = get_move_source(move);
    Square target_square = get_move_target(move);
    Piece piece = get_move_piece(move);
    Piece promoted = get_move_promoted(move);
    int capture_f = get_move_capture_f(move);
    int double_f = get_move_double_f(move);
    int enpassant_f = get_move_enpassant_f(move);
    int castling_f = get_move_castling_f(move);

    // move piece
    pop_bit(pos->bitboards[piece], source_square);
    set_bit(pos->bitboards[piece], target_square);
    pos->hash_key ^= piece_keys[piece][source_square];
    pos->hash_key ^= piece_keys[piece][target_square];
    pop_bit(pos->occupancies[pos->side], source_square);
    pop_bit(pos->occupancies[Color::both], source_square);
    set_bit(pos->occupancies[pos->side], target_square);
    set_bit(pos->occupancies[Color::both], target_square);

    // handle capture
    if (capture_f) {
      Piece start_piece = (pos->side == Color::white) ? Piece::p : Piece::P;
      Piece end_piece = (pos->side == Color::white) ? Piece::k : Piece::K;

      for (Piece bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
        if (get_bit(pos->bitboards[bb_piece], target_square)) {
          pop_bit(pos->bitboards[bb_piece], target_square);
          pos->hash_key ^= piece_keys[bb_piece][target_square];
          break;
        }
      }
      pop_bit(pos->occupancies[~pos->side], target_square);
    }

    // handle pawn promotion
    if (promoted) {
      set_bit(pos->bitboards[promoted], target_square);
      pop_bit(pos->bitboards[piece], target_square);
      pos->hash_key ^= piece_keys[promoted][target_square];
      pos->hash_key ^= piece_keys[piece][target_square];
    }

    // handle en passant
    if (enpassant_f) {
      if (pos->side == Color::white) {
        pop_bit(pos->bitboards[p], shift<DOWN>(target_square));
        pos->hash_key ^= piece_keys[p][shift<DOWN>(target_square)];
        pop_bit(pos->occupancies[Color::black], shift<DOWN>(target_square));
        pop_bit(pos->occupancies[Color::both], shift<DOWN>(target_square));
      }
      else {
        pop_bit(pos->bitboards[P], shift<UP>(target_square));
        pos->hash_key ^= piece_keys[P][shift<UP>(target_square)];
        pop_bit(pos->occupancies[Color::white], shift<UP>(target_square));
        pop_bit(pos->occupancies[Color::both], shift<UP>(target_square));
      }
    }

    // reset enpassant square
    if (pos->enpassant != Square::no_sq) {
      pos->hash_key ^= enpassant_keys[pos->enpassant];
      pos->enpassant = Square::no_sq;
    }

    // handle double pawn move
    if (double_f) {
      if (pos->side == Color::white) {
        pos->enpassant = shift<DOWN>(target_square);
      }
      else {
        pos->enpassant = shift<UP>(target_square);
      }
      pos->hash_key ^= enpassant_keys[pos->enpassant];
    }

    // handle castle
    if (castling_f) {
      // move rook
      switch (target_square)
      {
        case Square::g1:
          set_bit(pos->bitboards[Piece::R], Square::f1);
          pop_bit(pos->bitboards[Piece::R], Square::h1);
          pos->hash_key ^= piece_keys[Piece::R][Square::f1];
          pos->hash_key ^= piece_keys[Piece::R][Square::h1];
          set_bit(pos->occupancies[Color::white], Square::f1);
          pop_bit(pos->occupancies[Color::white], Square::h1);
          set_bit(pos->occupancies[Color::both], Square::f1);
          pop_bit(pos->occupancies[Color::both], Square::h1);
          break;
        case Square::c1:
          set_bit(pos->bitboards[Piece::R], Square::d1);
          pop_bit(pos->bitboards[Piece::R], Square::a1);
          pos->hash_key ^= piece_keys[Piece::R][Square::d1];
          pos->hash_key ^= piece_keys[Piece::R][Square::a1];
          set_bit(pos->occupancies[Color::white], Square::d1);
          pop_bit(pos->occupancies[Color::white], Square::a1);
          set_bit(pos->occupancies[Color::both], Square::d1);
          pop_bit(pos->occupancies[Color::both], Square::a1);
          break;
        case Square::g8:
          set_bit(pos->bitboards[Piece::r], Square::f8);
          pop_bit(pos->bitboards[Piece::r], Square::h8);
          pos->hash_key ^= piece_keys[Piece::r][Square::f8];
          pos->hash_key ^= piece_keys[Piece::r][Square::h8];
          set_bit(pos->occupancies[Color::black], Square::f8);
          pop_bit(pos->occupancies[Color::black], Square::h8);
          set_bit(pos->occupancies[Color::both], Square::f8);
          pop_bit(pos->occupancies[Color::both], Square::h8);
          break;
        case Square::c8:
          set_bit(pos->bitboards[Piece::r], Square::d8);
          pop_bit(pos->bitboards[Piece::r], Square::a8);
          pos->hash_key ^= piece_keys[Piece::r][Square::d8];
          pos->hash_key ^= piece_keys[Piece::r][Square::a8];
          set_bit(pos->occupancies[Color::black], Square::d8);
          pop_bit(pos->occupancies[Color::black], Square::a8);
          set_bit(pos->occupancies[Color::both], Square::d8);
          pop_bit(pos->occupancies[Color::both], Square::a8);
          break;
        }
    }

    // castling rights update
    pos->hash_key ^= castle_keys[pos->castle_rights];
    pos->castle_rights &= castling_rights[source_square];
    pos->castle_rights &= castling_rights[target_square];
    pos->hash_key ^= castle_keys[pos->castle_rights];

    // change side
    pos->side = ~pos->side;
    pos->hash_key ^= side_key;

    ++pos->ply;
    ++pos->repetition_index;
    pos->repetition_table[pos->repetition_index] = pos->hash_key;

    // make sure that king is not in check
    if (is_square_attacked(pos, (pos->side == Color::white) ? get_lsb_index(pos->bitboards[k]) : get_lsb_index(pos->bitboards[K]), pos->side)) {
      return 0;
    }
    else {
      return 1;
    }
  }
  else {
    // check if the move is a capture
    if (get_move_capture_f(move) || get_move_enpassant_f(move)) {
      return make_move(pos, move, Move_Type::all_moves);
    }
    else
      // don't make move
      return 0;
  }
}

void make_null_move(Position* pos) {
  pos->side = ~pos->side;
  pos->hash_key ^= side_key;

  if (pos->enpassant != Square::no_sq) {
    pos->hash_key ^= enpassant_keys[pos->enpassant];
    pos->enpassant = Square::no_sq;
  }

  ++pos->ply;
  ++pos->repetition_index;
  pos->repetition_table[pos->repetition_index] = pos->hash_key;
}

void print_board(Position* pos) {
  for (Rank rank = Rank::RANK_8; rank <= RANK_1; ++rank) {
    for (File file = File::FILE_A; file <= FILE_H; ++file) {
      Square square = get_square(rank, file);

      if (!file) {
        printf("  %d ", 8-rank);
      }

      Piece piece = Piece::no_p;

      for (Piece piece_bb = Piece::P; piece_bb <= Piece::k; ++piece_bb) {
        if (get_bit(pos->bitboards[piece_bb], square)) {
          piece = piece_bb; break;
        }
      }

      printf(" %c", (piece == Piece::no_p) ? '.' : ascii_pieces[piece]);
    }
    printf("\n");
  }

  printf("\n     a b c d e f g h\n\n");
  printf("     Side:     %s\n", (pos->side == Color::white) ? "white" : "black");
  printf("     En Passant:  %s\n", (pos->enpassant != Square::no_sq) ? square_to_coordinates[pos->enpassant] : "no");
  printf("     Casting:   %c%c%c%c\n\n", (pos->castle_rights & Castle_Right::wk) ? 'K' : '-',
                                         (pos->castle_rights & Castle_Right::wq) ? 'Q' : '-',
                                         (pos->castle_rights & Castle_Right::bk) ? 'k' : '-',
                                         (pos->castle_rights & Castle_Right::bq) ? 'q' : '-');
  printf("   Hash Key:  %llx\n", pos->hash_key);

}

void copy_position(Position* dst, Position* src) {
  memcpy(dst, src, sizeof(Position));
}