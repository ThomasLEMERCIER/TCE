#include "definition.hpp"

Color operator~(Color c) { return Color(c ^ 1); }

Direction operator~(Direction d) { return Direction(-d); };

Piece& operator++(Piece& piece) { return piece = Piece(int(piece) + 1); }
Piece& operator--(Piece& piece) { return piece = Piece(int(piece) - 1); }

Square& operator++(Square& s) { return s = Square(int(s) + 1); }
Square& operator--(Square& s) { return s = Square(int(s) - 1); }
Square operator+(Square s, int i) { return Square(int(s) + i); }
Square operator-(Square s, int i) { return Square(int(s) - i); }
Square& operator+=(Square& s, int i) { return s = Square(int(s) + i); }
Square& operator-=(Square& s, int i) { return s = Square(int(s) - i); }


File& operator++(File& f) { return f = File(int(f) + 1); }
File& operator--(File& f) { return f = File(int(f) - 1); }

Rank& operator++(Rank& r) { return r = Rank(int(r) + 1); }
Rank& operator--(Rank& r) { return r = Rank(int(r) - 1); }

Square get_square(Rank r, File f) { return Square(8 * r + f); }
File file_of(Square s) { return File(s & 7); }
Rank rank_of(Square s) { return Rank(s >> 3); }
