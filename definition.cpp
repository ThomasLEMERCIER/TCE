#include "definition.hpp"

Color operator~(Color c) { return Color(c ^ 1); }

Piece& operator++(Piece& d) { return d = Piece(int(d) + 1); }
Piece& operator--(Piece& d) { return d = Piece(int(d) - 1); }

Square& operator++(Square& s) { return s = Square(int(s) + 1); }
Square& operator--(Square& s) { return s = Square(int(s) - 1); }

File& operator++(File& d) { return d = File(int(d) + 1); }
File& operator--(File& d) { return d = File(int(d) - 1); }

Rank& operator++(Rank& s) { return s = Rank(int(s) + 1); }
Rank& operator--(Rank& s) { return s = Rank(int(s) - 1); }

Square get_square(Rank r, File f) { return Square(8 * r + f); }
File file_of(Square s) { return File(s & 7); }
Rank rank_of(Square s) { return Rank(s >> 3); }
