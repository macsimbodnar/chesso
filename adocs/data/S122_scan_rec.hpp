#pragma once
// S122's fast check and the scans DEC-344's Context rests on: one fixed record a corpus
// row, what S122_scan_dump.cpp writes and S122_scans.cpp reads, so that a grid over labels
// costs arithmetic and not a board. Counts are S122_census_core.hpp's, each indexed by the
// colour of the king they are about, so [1] is what White's men do to Black's king.
#include <cstdint>

struct scan_rec_t
{
  float result;  // White's point of view
  int32_t game;
  int16_t white_eval;  // evaluate(), White relative
  int16_t two_white;   // stage two before the clamp, White relative
  int16_t score;       // the search's, White relative
  uint8_t phase;
  uint8_t stm_white;
  uint8_t u[2];  // ks_units()
  uint8_t att[2], inc[2];
  uint8_t chk[2];   // reading 2, men included
  uint8_t chke[2];  // reading 2, empty target squares only
  uint8_t weak[2];
  uint8_t defsq[2];
  uint8_t an[2], ab[2], ar[2], aq[2];
  uint8_t cn[2], cb[2], cr[2], cq[2];  // reading 2 by type
  uint8_t graded;                      // bit 0 White's king, bit 1 Black's: >= 3 attackers, a queen, a safe check
  uint8_t pad[3];
};
