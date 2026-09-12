#pragma once
#include <array>
#include <bit>
#include "data_structures.hpp"


//-#####################  THE GEOMETRY OF A SQUARE  #########################-//
// S211. Every table further down that is not a magic is derived here rather
// than typed, so the only board knowledge in this file is the four lines
// below and the step lists that follow them.
//
// A square is `row * 8 + col`. Row 0 is the eighth rank and col 0 the a-file,
// because bb_squares_t counts from a8, so one step of (dr, dc) lands on
// `(row + dr) * 8 + (col + dc)`. Every bounds test in this file is on the two
// coordinates and never on a shifted bitboard: a move that leaves the board
// fails the test instead of reappearing on the opposite file, so there is no
// wrap to mask away afterwards and no mask to get the wrong way round.
constexpr int square_row(int square)
{ return square / 8; }
constexpr int square_col(int square)
{ return square % 8; }
constexpr int square_at(int row, int col)
{ return (row * 8) + col; }

constexpr bool square_on_board(int row, int col)
{ return (row >= 0) && (row < 8) && (col >= 0) && (col < 8); }


// One move of a piece, as the row and column it shifts by.
struct step_t
{
  int dr;
  int dc;
};

// The directions a slider travels: a bishop changes both coordinates by one, a
// rook exactly one of them. Listed in (dr, dc) order, which is an arbitrary
// order -- nothing below depends on it, because every ray is walked to its own
// end and the four results are unioned.
static inline constexpr step_t BISHOP_STEPS[4] = {{-1, -1},
                                                  {-1, 1},
                                                  {1, -1},
                                                  {1, 1}};

static inline constexpr step_t ROOK_STEPS[4] = {{-1, 0},
                                                {0, -1},
                                                {0, 1},
                                                {1, 0}};


// What a slider attacks along one direction: every square it crosses, plus the
// first occupied one, which is where the ray ends because that square is the
// capture and nothing behind it is reachable.
constexpr bb_t ray_attacks(int square, step_t step, bb_t blockers)
{
  bb_t ray = BB_0;
  int row = square_row(square) + step.dr;
  int col = square_col(square) + step.dc;

  while (square_on_board(row, col)) {
    const bb_t reached = BB_1 << square_at(row, col);
    ray |= reached;

    if (reached & blockers) { break; }

    row += step.dr;
    col += step.dc;
  }

  return ray;
}


// The squares on one ray whose occupancy can change the answer above. A piece
// on the last square of a ray shadows nothing -- there is no further square to
// hide -- so the attack set is the same whether it stands there or not, and
// the square is left out. A square is in, then, exactly when one more step in
// the same direction is still on the board.
//
// Leaving those four squares out is what makes the magic hashing possible at
// all: it is the difference between a rook index of up to 14 bits and one of
// up to 12, and so between tables of 16384 and 4096 entries per square.
constexpr bb_t ray_relevant(int square, step_t step)
{
  bb_t ray = BB_0;
  int row = square_row(square) + step.dr;
  int col = square_col(square) + step.dc;

  while (square_on_board(row + step.dr, col + step.dc)) {
    ray |= BB_1 << square_at(row, col);
    row += step.dr;
    col += step.dc;
  }

  return ray;
}


constexpr bb_t bishop_attacks_from(int square, bb_t blockers)
{
  bb_t attacks = BB_0;
  for (const step_t& step : BISHOP_STEPS) {
    attacks |= ray_attacks(square, step, blockers);
  }
  return attacks;
}


constexpr bb_t rook_attacks_from(int square, bb_t blockers)
{
  bb_t attacks = BB_0;
  for (const step_t& step : ROOK_STEPS) {
    attacks |= ray_attacks(square, step, blockers);
  }
  return attacks;
}


constexpr bb_t bishop_relevant_mask(int square)
{
  bb_t mask = BB_0;
  for (const step_t& step : BISHOP_STEPS) {
    mask |= ray_relevant(square, step);
  }
  return mask;
}


constexpr bb_t rook_relevant_mask(int square)
{
  bb_t mask = BB_0;
  for (const step_t& step : ROOK_STEPS) {
    mask |= ray_relevant(square, step);
  }
  return mask;
}


//-########################  THE DERIVED TABLES  ############################-//

// How many bits a square's occupancy index needs: the number of squares in its
// relevant mask. std::popcount and not count_bits() only because bitboard.hpp,
// where count_bits() lives, includes this file and not the other way round --
// count_bits() is that same call. Both tables come out of one walk, so the
// geometry above cannot move one of them without moving the other.
constexpr std::array<uint8_t, 64> relevant_bits_table(bool rook)
{
  std::array<uint8_t, 64> counts = {};

  for (int square = 0; square < 64; ++square) {
    const bb_t mask =
        rook ? rook_relevant_mask(square) : bishop_relevant_mask(square);
    counts[static_cast<size_t>(square)] =
        static_cast<uint8_t>(std::popcount(mask));
  }

  return counts;
}

static inline constexpr std::array<uint8_t, 64> bishop_relevant_bits_count =
    relevant_bits_table(false);

static inline constexpr std::array<uint8_t, 64> rook_relevant_bits_count =
    relevant_bits_table(true);


// Which castling rights survive a move that touches a square. Six squares
// carry a right and the other 58 carry none, so the table starts with all four
// rights everywhere and clears what each of the six destroys: a king's square
// loses both rights of its colour, a rook's corner loses the right on its own
// wing. make_move ANDs it against both the from-square and the to-square,
// which is what makes one table cover three different losses -- the king
// moving, the rook moving, and the rook being captured where it stands.
constexpr std::array<castling_t, 64> castling_rights_table()
{
  std::array<castling_t, 64> rights = {};
  for (castling_t& entry : rights) {
    entry = static_cast<castling_t>(WK | WQ | BK | BQ);
  }

  rights[e1] = static_cast<castling_t>(rights[e1] & ~(WK | WQ));
  rights[h1] = static_cast<castling_t>(rights[h1] & ~WK);
  rights[a1] = static_cast<castling_t>(rights[a1] & ~WQ);

  rights[e8] = static_cast<castling_t>(rights[e8] & ~(BK | BQ));
  rights[h8] = static_cast<castling_t>(rights[h8] & ~BK);
  rights[a8] = static_cast<castling_t>(rights[a8] & ~BQ);

  return rights;
}

static inline constexpr std::array<castling_t, 64> castling_rights =
    castling_rights_table();


// Provenance, S179 under DEC-132. Both arrays below are this project's own
// output, drawn under seed 20260904 -- the date of DEC-132, the decision that
// asked for a seed of the project's own. Regenerate with:
//
//   ./build/tools/magic_gen magics --seed 20260904
//
// then paste both arrays here and run ./clang-format.sh. The relevant-bit
// counts above, the shifts in get_rook_attacks / get_bishop_attacks and the
// attack-table layout in bb_tables_t are unchanged by that command: a magic is
// a perfect hash into a table whose size is fixed by the relevant-bit count, so
// a different valid set moves which slot an occupancy lands in and nothing the
// move generator returns. The set these replaced was generated here in 2023
// (a5dbe68) under the seed a public tutorial series uses, and coincided with
// that series' published set value for value; the new set shares 0 of its 128
// values with it.
static inline constexpr bb_t rook_magic_numbers[64] = {
    0x2080002040001080ULL, 0x440004020001000ULL,  0x80200010008008ULL,
    0x80100080080004ULL,   0x1080040108008036ULL, 0x8600102814020001ULL,
    0x400045208010090ULL,  0xe080042100114080ULL, 0x802040008003ULL,
    0x2402000401000ULL,    0x20801004842000ULL,   0x182801000480081ULL,
    0x8800800040080ULL,    0x5800400810200ULL,    0x800200010080ULL,
    0x2000210411e84ULL,    0x840008004c12680ULL,  0x800908020004000ULL,
    0x9010048020008110ULL, 0x1090021041000ULL,    0x2010828008004400ULL,
    0x400080104402010ULL,  0x6100400900d4218ULL,  0x120000805104ULL,
    0x9090400080082080ULL, 0x400400100210080ULL,  0x804200201200ULL,
    0x8818028080081002ULL, 0x40080280040080ULL,   0x8002000200100408ULL,
    0x800010400420850ULL,  0xd0200209044ULL,      0x2488401820800082ULL,
    0x20004000802082ULL,   0x4411002001001041ULL, 0x25001001000820ULL,
    0x28040080800800ULL,   0x24008044800200ULL,   0x50011004000288ULL,
    0x802008042000104ULL,  0x23984c0008000ULL,    0x8c10004020004000ULL,
    0x100e814012020021ULL, 0x4010040811010020ULL, 0x84001008010100ULL,
    0x9000804010002ULL,    0x34020001008080ULL,   0x207010040820004ULL,
    0x28080004300a300ULL,  0x600080400180ULL,     0xa020004010080040ULL,
    0x1c4011a08a0200ULL,   0x812001004200a00ULL,  0x9104800200040080ULL,
    0x40001002c8010400ULL, 0x103041c824200ULL,    0x4201022442801202ULL,
    0x40810020104001ULL,   0x8600100840820022ULL, 0xc045001000a00409ULL,
    0x1882000420081002ULL, 0x8a09000400020801ULL, 0x60030a238100104ULL,
    0x1205c081040122ULL,
};

static inline constexpr bb_t bishop_magic_numbers[64] = {
    0x4ac0210111020883ULL, 0x80428382d102081ULL,  0x108520042100081ULL,
    0x88208022004004ULL,   0x8402021002001001ULL, 0x901008402208ULL,
    0x2182080108080080ULL, 0x8000120201202848ULL, 0x88122004306a0258ULL,
    0x202104048280ULL,     0x86090a400852000ULL,  0x49202000440ULL,
    0x6a00040420202000ULL, 0x10020210040006ULL,   0x820010918600421ULL,
    0x400020205098840ULL,  0x220084002c20a00ULL,  0xa00048050c5180ULL,
    0x30204002080301a0ULL, 0x108030082004400ULL,  0x4004202a20800ULL,
    0x304502020100a200ULL, 0xa0041120a122101ULL,  0x900042080141ULL,
    0x482416008084840ULL,  0x14902204a0400ULL,    0x2082220410208200ULL,
    0x24010010200880ULL,   0x1001007004000ULL,    0x1010140800808404ULL,
    0x1042042022010100ULL, 0xa004042029090100ULL, 0x1a80800409010ULL,
    0x4065192010080800ULL, 0x4280184800440800ULL, 0x8004020084580080ULL,
    0x2020804240040ULL,    0x8104082420420088ULL, 0x8008482840008200ULL,
    0x4140030004104ULL,    0x400801249020e001ULL, 0x90c02e404003042ULL,
    0x82082488001000ULL,   0x884200801800ULL,     0xc84621040400400ULL,
    0x50011200b2000100ULL, 0x410160800500308ULL,  0x10100a0080204118ULL,
    0x401042202420461ULL,  0x2002088084102090ULL, 0x21d00a0084040620ULL,
    0x28000104882800ULL,   0xe02001202021000ULL,  0x40081010808040ULL,
    0x440044102460802ULL,  0xc41010810040800aULL, 0x222010402020220ULL,
    0x80055c2084442000ULL, 0x58084c020ac1010ULL,  0x404040049208848ULL,
    0x2040032020202480ULL, 0x3010402820880490ULL, 0x4220082001840100ULL,
    0x20510948008080ULL,
};

static inline constexpr bb_t file_masks[64] = {
    0x0101010101010101, 0x0202020202020202, 0x0404040404040404,
    0x0808080808080808, 0x1010101010101010, 0x2020202020202020,
    0x4040404040404040, 0x8080808080808080, 0x0101010101010101,
    0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040,
    0x8080808080808080, 0x0101010101010101, 0x0202020202020202,
    0x0404040404040404, 0x0808080808080808, 0x1010101010101010,
    0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
    0x0101010101010101, 0x0202020202020202, 0x0404040404040404,
    0x0808080808080808, 0x1010101010101010, 0x2020202020202020,
    0x4040404040404040, 0x8080808080808080, 0x0101010101010101,
    0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040,
    0x8080808080808080, 0x0101010101010101, 0x0202020202020202,
    0x0404040404040404, 0x0808080808080808, 0x1010101010101010,
    0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
    0x0101010101010101, 0x0202020202020202, 0x0404040404040404,
    0x0808080808080808, 0x1010101010101010, 0x2020202020202020,
    0x4040404040404040, 0x8080808080808080, 0x0101010101010101,
    0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040,
    0x8080808080808080,
};

static inline constexpr bb_t rank_masks[64] = {
    0x00000000000000ff, 0x00000000000000ff, 0x00000000000000ff,
    0x00000000000000ff, 0x00000000000000ff, 0x00000000000000ff,
    0x00000000000000ff, 0x00000000000000ff, 0x000000000000ff00,
    0x000000000000ff00, 0x000000000000ff00, 0x000000000000ff00,
    0x000000000000ff00, 0x000000000000ff00, 0x000000000000ff00,
    0x000000000000ff00, 0x0000000000ff0000, 0x0000000000ff0000,
    0x0000000000ff0000, 0x0000000000ff0000, 0x0000000000ff0000,
    0x0000000000ff0000, 0x0000000000ff0000, 0x0000000000ff0000,
    0x00000000ff000000, 0x00000000ff000000, 0x00000000ff000000,
    0x00000000ff000000, 0x00000000ff000000, 0x00000000ff000000,
    0x00000000ff000000, 0x00000000ff000000, 0x000000ff00000000,
    0x000000ff00000000, 0x000000ff00000000, 0x000000ff00000000,
    0x000000ff00000000, 0x000000ff00000000, 0x000000ff00000000,
    0x000000ff00000000, 0x0000ff0000000000, 0x0000ff0000000000,
    0x0000ff0000000000, 0x0000ff0000000000, 0x0000ff0000000000,
    0x0000ff0000000000, 0x0000ff0000000000, 0x0000ff0000000000,
    0x00ff000000000000, 0x00ff000000000000, 0x00ff000000000000,
    0x00ff000000000000, 0x00ff000000000000, 0x00ff000000000000,
    0x00ff000000000000, 0x00ff000000000000, 0xff00000000000000,
    0xff00000000000000, 0xff00000000000000, 0xff00000000000000,
    0xff00000000000000, 0xff00000000000000, 0xff00000000000000,
    0xff00000000000000,
};

static inline constexpr bb_t isolated_file_masks[64] = {
    0x0202020202020202, 0x0505050505050505, 0x0a0a0a0a0a0a0a0a,
    0x1414141414141414, 0x2828282828282828, 0x5050505050505050,
    0xa0a0a0a0a0a0a0a0, 0x4040404040404040, 0x0202020202020202,
    0x0505050505050505, 0x0a0a0a0a0a0a0a0a, 0x1414141414141414,
    0x2828282828282828, 0x5050505050505050, 0xa0a0a0a0a0a0a0a0,
    0x4040404040404040, 0x0202020202020202, 0x0505050505050505,
    0x0a0a0a0a0a0a0a0a, 0x1414141414141414, 0x2828282828282828,
    0x5050505050505050, 0xa0a0a0a0a0a0a0a0, 0x4040404040404040,
    0x0202020202020202, 0x0505050505050505, 0x0a0a0a0a0a0a0a0a,
    0x1414141414141414, 0x2828282828282828, 0x5050505050505050,
    0xa0a0a0a0a0a0a0a0, 0x4040404040404040, 0x0202020202020202,
    0x0505050505050505, 0x0a0a0a0a0a0a0a0a, 0x1414141414141414,
    0x2828282828282828, 0x5050505050505050, 0xa0a0a0a0a0a0a0a0,
    0x4040404040404040, 0x0202020202020202, 0x0505050505050505,
    0x0a0a0a0a0a0a0a0a, 0x1414141414141414, 0x2828282828282828,
    0x5050505050505050, 0xa0a0a0a0a0a0a0a0, 0x4040404040404040,
    0x0202020202020202, 0x0505050505050505, 0x0a0a0a0a0a0a0a0a,
    0x1414141414141414, 0x2828282828282828, 0x5050505050505050,
    0xa0a0a0a0a0a0a0a0, 0x4040404040404040, 0x0202020202020202,
    0x0505050505050505, 0x0a0a0a0a0a0a0a0a, 0x1414141414141414,
    0x2828282828282828, 0x5050505050505050, 0xa0a0a0a0a0a0a0a0,
    0x4040404040404040,
};

static inline constexpr bb_t passed_w_pawns_masks[64] = {
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000003ULL,
    0x0000000000000007ULL, 0x000000000000000eULL, 0x000000000000001cULL,
    0x0000000000000038ULL, 0x0000000000000070ULL, 0x00000000000000e0ULL,
    0x00000000000000c0ULL, 0x0000000000000303ULL, 0x0000000000000707ULL,
    0x0000000000000e0eULL, 0x0000000000001c1cULL, 0x0000000000003838ULL,
    0x0000000000007070ULL, 0x000000000000e0e0ULL, 0x000000000000c0c0ULL,
    0x0000000000030303ULL, 0x0000000000070707ULL, 0x00000000000e0e0eULL,
    0x00000000001c1c1cULL, 0x0000000000383838ULL, 0x0000000000707070ULL,
    0x0000000000e0e0e0ULL, 0x0000000000c0c0c0ULL, 0x0000000003030303ULL,
    0x0000000007070707ULL, 0x000000000e0e0e0eULL, 0x000000001c1c1c1cULL,
    0x0000000038383838ULL, 0x0000000070707070ULL, 0x00000000e0e0e0e0ULL,
    0x00000000c0c0c0c0ULL, 0x0000000303030303ULL, 0x0000000707070707ULL,
    0x0000000e0e0e0e0eULL, 0x0000001c1c1c1c1cULL, 0x0000003838383838ULL,
    0x0000007070707070ULL, 0x000000e0e0e0e0e0ULL, 0x000000c0c0c0c0c0ULL,
    0x0000030303030303ULL, 0x0000070707070707ULL, 0x00000e0e0e0e0e0eULL,
    0x00001c1c1c1c1c1cULL, 0x0000383838383838ULL, 0x0000707070707070ULL,
    0x0000e0e0e0e0e0e0ULL, 0x0000c0c0c0c0c0c0ULL, 0x0003030303030303ULL,
    0x0007070707070707ULL, 0x000e0e0e0e0e0e0eULL, 0x001c1c1c1c1c1c1cULL,
    0x0038383838383838ULL, 0x0070707070707070ULL, 0x00e0e0e0e0e0e0e0ULL,
    0x00c0c0c0c0c0c0c0ULL,
};

static inline constexpr bb_t passed_b_pawns_masks[64] = {
    0x0303030303030300ULL, 0x0707070707070700ULL, 0x0e0e0e0e0e0e0e00ULL,
    0x1c1c1c1c1c1c1c00ULL, 0x3838383838383800ULL, 0x7070707070707000ULL,
    0xe0e0e0e0e0e0e000ULL, 0xc0c0c0c0c0c0c000ULL, 0x0303030303030000ULL,
    0x0707070707070000ULL, 0x0e0e0e0e0e0e0000ULL, 0x1c1c1c1c1c1c0000ULL,
    0x3838383838380000ULL, 0x7070707070700000ULL, 0xe0e0e0e0e0e00000ULL,
    0xc0c0c0c0c0c00000ULL, 0x0303030303000000ULL, 0x0707070707000000ULL,
    0x0e0e0e0e0e000000ULL, 0x1c1c1c1c1c000000ULL, 0x3838383838000000ULL,
    0x7070707070000000ULL, 0xe0e0e0e0e0000000ULL, 0xc0c0c0c0c0000000ULL,
    0x0303030300000000ULL, 0x0707070700000000ULL, 0x0e0e0e0e00000000ULL,
    0x1c1c1c1c00000000ULL, 0x3838383800000000ULL, 0x7070707000000000ULL,
    0xe0e0e0e000000000ULL, 0xc0c0c0c000000000ULL, 0x0303030000000000ULL,
    0x0707070000000000ULL, 0x0e0e0e0000000000ULL, 0x1c1c1c0000000000ULL,
    0x3838380000000000ULL, 0x7070700000000000ULL, 0xe0e0e00000000000ULL,
    0xc0c0c00000000000ULL, 0x0303000000000000ULL, 0x0707000000000000ULL,
    0x0e0e000000000000ULL, 0x1c1c000000000000ULL, 0x3838000000000000ULL,
    0x7070000000000000ULL, 0xe0e0000000000000ULL, 0xc0c0000000000000ULL,
    0x0300000000000000ULL, 0x0700000000000000ULL, 0x0e00000000000000ULL,
    0x1c00000000000000ULL, 0x3800000000000000ULL, 0x7000000000000000ULL,
    0xe000000000000000ULL, 0xc000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
    0x0000000000000000ULL,
};
