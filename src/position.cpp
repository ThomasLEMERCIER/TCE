#include "position.hpp"

#include "rng.hpp"
#include "move.hpp"

#include <sstream>
#include <cstring>    // memcpy memset
#include <iostream>

U64 piece_keys[12][64];
U64 side_key;
U64 enpassant_keys[64];
U64 castle_keys[64];

Position::Position(Position* pos) {
  memcpy(bitboards, pos->bitboards, sizeof(bitboards));
  memcpy(occupancies, pos->occupancies, sizeof(occupancies));
  memcpy(repetition_table, pos->repetition_table, sizeof(repetition_table));
  
  side = pos->side;
  enpassant = pos->enpassant;
  castle_rights = pos->castle_rights;

  hash_key = pos->hash_key;
  repetition_index = pos->repetition_index;
  ply = pos->ply;
};

void Position::set(const std::string& fenStr) {
  memset(this, 0, sizeof(Position));

  std::istringstream ss(fenStr);
  ss >> std::noskipws;
  unsigned char token;

  Square square = Square::FIRST_SQUARE;
  Piece piece;

  // parse piece placement
  while ((ss >> token) && token != ' ') {
    if (token == '/')         continue;
    else if (isdigit(token))  square += (token - '0');
    else {
      piece = char_pieces.at(token);
      set_bit(bitboards[piece], square);
      ++square;
    }
  }

  ss >> token;
  side = (token == 'w') ? Color::WHITE : Color::BLACK;

  ss >> token;
  while ((ss >> token) && token != ' ') {
    switch (token)
    {
      case 'K': castle_rights |= Castle_Right::WOO; break;
      case 'Q': castle_rights |= Castle_Right::WOOO; break;
      case 'k': castle_rights |= Castle_Right::BOO; break;
      case 'q': castle_rights |= Castle_Right::BOOO; break;
      default: break;
    }
  }

  ss >> token;
  if (token == '-') {
    enpassant = Square::NO_SQUARE;
  }
  else {
    int file = token - 'a';
    ss >> token;
    int rank = 8 - (token - '0');
    enpassant = get_square(Rank(rank), File(file));
  }

  // loop over white pieces bitboards
  for (piece = Piece::WP; piece <= Piece::WK; ++piece)
    occupancies[Color::WHITE] |= bitboards[piece];
  // loop over black pieces bitboards
  for (piece = Piece::BP; piece <= Piece::BK; ++piece)
    occupancies[Color::BLACK] |= bitboards[piece];

  occupancies[Color::BOTH] |= occupancies[Color::WHITE];
  occupancies[Color::BOTH] |= occupancies[Color::BLACK];

  hash_key = generate_hash_key(this);

  repetition_index = 0;
  repetition_table[repetition_index] = hash_key;
}

bool Position::is_repetition() {
  if (!ply) return false;
  for (int index = 0; index < repetition_index; index++) {
    if (repetition_table[index] == hash_key)
      return true;
  }
  return false;
}

U64 generate_hash_key(Position* pos) {
  U64 final_key = 0ULL;

  // loop over piece bitboards
  for (Piece piece = Piece::WP; piece <= Piece::BK; ++piece) {
    Bitboard bitboard = pos->bitboards[piece];
    while (bitboard) {
      Square square = get_lsb_index(bitboard);
      
      final_key ^= piece_keys[piece][square];

      pop_bit(bitboard, square);
    }
  }
  
  // if enpassant square is on board
  if (pos->enpassant != Square::NO_SQUARE)
    final_key ^= enpassant_keys[pos->enpassant];
  
  // castling hash
  final_key ^= castle_keys[pos->castle_rights];
  
  // hash the side only if black is to move
  if (pos->side == Color::BLACK) final_key ^= side_key;

  return final_key;
};

void init_random_keys() {
  RNG rng((unsigned long)1804289383);

  // loop over pieces and squares
  for (Piece piece = Piece::WP; piece <= Piece::BK; ++piece) {
    for (Square square = Square::FIRST_SQUARE; square <= Square::LAST_SQUARE; ++square)
      // init random piece keys
      piece_keys[piece][square] = rng.rand64();
  }
  
  // loop over squares
  for (Square square = Square::FIRST_SQUARE; square <= Square::LAST_SQUARE; ++square)
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
  // using symmetry of attack pattern

  // white pawn attack
  if ((side == Color::WHITE) && (pawn_attacks[Color::BLACK][square] & pos->bitboards[WP])) return 1;

  // black pawn attack
  if ((side == Color::BLACK) && (pawn_attacks[Color::WHITE][square] & pos->bitboards[BP])) return 1;

  // knight attack
  if (knight_attacks[square] & ((side == Color::WHITE) ? pos->bitboards[WN] :pos-> bitboards[BN])) return 1;

  // bishop attack
  if (get_bishop_attacks(square, pos->occupancies[Color::BOTH]) & ((side == Color::WHITE) ? pos->bitboards[WB] : pos->bitboards[BB])) return 1;

  // bishop attack
  if (get_rook_attacks(square, pos->occupancies[Color::BOTH]) & ((side == Color::WHITE) ? pos->bitboards[WR] : pos->bitboards[BR])) return 1;

  // queen attack
  if (get_queen_attacks(square, pos->occupancies[Color::BOTH]) & ((side == Color::WHITE) ? pos->bitboards[WQ] : pos->bitboards[BQ])) return 1;

  // king attack
  if (king_attacks[square] & ((side == Color::WHITE) ? pos->bitboards[WK] : pos->bitboards[BK])) return 1;

  return 0;
}


int make_move(Position* pos, Move move, Move_Type move_flag) {
  if (move_flag == Move_Type::ALL_MOVES) {
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
    pop_bit(pos->occupancies[Color::BOTH], source_square);
    set_bit(pos->occupancies[pos->side], target_square);
    set_bit(pos->occupancies[Color::BOTH], target_square);

    // handle capture
    if (capture_f) {
      Piece start_piece = (pos->side == Color::WHITE) ? Piece::BP : Piece::WP;
      Piece end_piece = (pos->side == Color::WHITE) ? Piece::BK : Piece::WK;

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
      if (pos->side == Color::WHITE) {
        pop_bit(pos->bitboards[BP], shift<DOWN>(target_square));
        pos->hash_key ^= piece_keys[BP][shift<DOWN>(target_square)];
        pop_bit(pos->occupancies[Color::BLACK], shift<DOWN>(target_square));
        pop_bit(pos->occupancies[Color::BOTH], shift<DOWN>(target_square));
      }
      else {
        pop_bit(pos->bitboards[WP], shift<UP>(target_square));
        pos->hash_key ^= piece_keys[WP][shift<UP>(target_square)];
        pop_bit(pos->occupancies[Color::WHITE], shift<UP>(target_square));
        pop_bit(pos->occupancies[Color::BOTH], shift<UP>(target_square));
      }
    }

    // reset enpassant square
    if (pos->enpassant != Square::NO_SQUARE) {
      pos->hash_key ^= enpassant_keys[pos->enpassant];
      pos->enpassant = Square::NO_SQUARE;
    }

    // handle double pawn move
    if (double_f) {
      if (pos->side == Color::WHITE) {
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
        case Square::G1:
          set_bit(pos->bitboards[Piece::WR], Square::F1);
          pop_bit(pos->bitboards[Piece::WR], Square::H1);
          pos->hash_key ^= piece_keys[Piece::WR][Square::F1];
          pos->hash_key ^= piece_keys[Piece::WR][Square::H1];
          set_bit(pos->occupancies[Color::WHITE], Square::F1);
          pop_bit(pos->occupancies[Color::WHITE], Square::H1);
          set_bit(pos->occupancies[Color::BOTH], Square::F1);
          pop_bit(pos->occupancies[Color::BOTH], Square::H1);
          break;
        case Square::C1:
          set_bit(pos->bitboards[Piece::WR], Square::D1);
          pop_bit(pos->bitboards[Piece::WR], Square::A1);
          pos->hash_key ^= piece_keys[Piece::WR][Square::D1];
          pos->hash_key ^= piece_keys[Piece::WR][Square::A1];
          set_bit(pos->occupancies[Color::WHITE], Square::D1);
          pop_bit(pos->occupancies[Color::WHITE], Square::A1);
          set_bit(pos->occupancies[Color::BOTH], Square::D1);
          pop_bit(pos->occupancies[Color::BOTH], Square::A1);
          break;
        case Square::G8:
          set_bit(pos->bitboards[Piece::BR], Square::F8);
          pop_bit(pos->bitboards[Piece::BR], Square::H8);
          pos->hash_key ^= piece_keys[Piece::BR][Square::F8];
          pos->hash_key ^= piece_keys[Piece::BR][Square::H8];
          set_bit(pos->occupancies[Color::BLACK], Square::F8);
          pop_bit(pos->occupancies[Color::BLACK], Square::H8);
          set_bit(pos->occupancies[Color::BOTH], Square::F8);
          pop_bit(pos->occupancies[Color::BOTH], Square::H8);
          break;
        case Square::C8:
          set_bit(pos->bitboards[Piece::BR], Square::D8);
          pop_bit(pos->bitboards[Piece::BR], Square::A8);
          pos->hash_key ^= piece_keys[Piece::BR][Square::D8];
          pos->hash_key ^= piece_keys[Piece::BR][Square::A8];
          set_bit(pos->occupancies[Color::BLACK], Square::D8);
          pop_bit(pos->occupancies[Color::BLACK], Square::A8);
          set_bit(pos->occupancies[Color::BOTH], Square::D8);
          pop_bit(pos->occupancies[Color::BOTH], Square::A8);
          break;
        default:
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
    if (is_square_attacked(pos, (pos->side == Color::WHITE) ? get_lsb_index(pos->bitboards[BK]) : get_lsb_index(pos->bitboards[WK]), pos->side)) {
      return 0;
    }
    else {
      return 1;
    }
  }
  else {
    // check if the move is a capture
    if (get_move_capture_f(move) || get_move_enpassant_f(move)) {
      return make_move(pos, move, Move_Type::ALL_MOVES);
    }
    else
      // don't make move
      return 0;
  }
}

void make_null_move(Position* pos) {
  pos->side = ~pos->side;
  pos->hash_key ^= side_key;

  if (pos->enpassant != Square::NO_SQUARE) {
    pos->hash_key ^= enpassant_keys[pos->enpassant];
    pos->enpassant = Square::NO_SQUARE;
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
        std::cout << "  " << 8 - rank << " ";
      }

      Piece piece = Piece::NO_PIECE;

      for (Piece piece_bb = Piece::WP; piece_bb <= Piece::BK; ++piece_bb) {
        if (get_bit(pos->bitboards[piece_bb], square)) {
          piece = piece_bb; break;
        }
      }
      std::cout << " " << ((piece == Piece::NO_PIECE) ? '.' : ascii_pieces[piece]);
    }
    std::cout << '\n';
  }


  std::cout << "\n     a b c d e f g h\n\n";
  std::cout << "     Side:     " << ((pos->side == Color::WHITE) ? "white" : "black") << '\n';
  std::cout << "     En Passant:  " << ((pos->enpassant != Square::NO_SQUARE) ? square_to_coordinates[pos->enpassant] : "no") << '\n';
  std::cout << "     Casting:   " << ((pos->castle_rights & Castle_Right::WOO) ? 'K' : '-') << ((pos->castle_rights & Castle_Right::WOOO) ? 'Q' : '-') << ((pos->castle_rights & Castle_Right::BOO) ? 'k' : '-') << ((pos->castle_rights & Castle_Right::BOOO) ? 'q' : '-') << '\n';
  std::cout << "\n   Hash Key:  " << std::hex << pos->hash_key << std::dec << std::endl;

}

void copy_position(Position* dst, Position* src) {
  memcpy(dst, src, sizeof(Position));
}