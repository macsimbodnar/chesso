// Fitted by tools/tuner from 11003693 self-play positions.
// data       /home/max/ws/chesso/.tuning/selfplay_v2.tsv
// K          0.7595
// error      0.074006 train, 0.074249 validation
// seed       1, lr 1.000, validation split 0.10
// split      by game, S066: 119998 games, 1100388 rows held out, 10.0002%
// only       all
// freeze     tempo,piece_placement
// lambda     0.2500  target = lambda * sigma(K * score) + (1 - lambda) * result
// wdl error  0.118457 validation, against 0.118457 at the constants this started
//            from. This selects lambda; loss is not Elo and an SPRT decides.
//
// Paste over the corresponding definitions. The piece defines and the
// two tables live in src/eval_tables.hpp; the mobility, king safety,
// passed pawn, pawn structure, piece placement and tempo weights at
// the end live in src/evaluation.cpp, a different file and easy to
// miss.
// S027, S028, S034 and S035, DEC-015: measured by SPRT before any of
// it is kept.

#define PAWN    95
#define KNIGHT  327
#define BISHOP  306
#define ROOK    487
#define QUEEN   716

static constexpr int psqt_mg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     255,  129,  102,  217,  149,   22,   -5,  113,
     -28,  -22,    4,   25,   19,   15,  -24,   21,
     -37,   17,    6,    8,   16,    6,   27,  -29,
     -38,  -27,    0,    3,    4,  -10,  -10,  -33,
     -18,  -24,   -3,  -31,  -21,  -30,   20,  -14,
     -42,  -17,  -25,  -66,  -59,  -24,   18,  -29,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -183, -177, -125, -124,    2, -244,   82,  -70,
     -17,  -56,   85, -100,  -49,   89,  -71,   12,
      17,   87,  -15,  109,   87,   75,   84,   10,
      -2,   39,   52,   66,   39,   73,   30,   55,
      13,  -24,   39,   25,   37,   63,   29,   24,
     -26,    7,   24,   46,   46,   42,   48,   -6,
     -34,  -26,   12,   16,   25,   23,   19,    0,
    -137,  -30,  -49,  -41,    8,  -19,  -20, -119
  },
  {  // bishop
      33,  -82, -234, -209,  -66, -263,  -66,   74,
      18,   44,   36,  -14, -164,   59,   11,   53,
     -57,   46,   12,   38,   66, -157,   94,   14,
      32,   70,   49,   84,   58,   35,   61,   67,
      81,   10,   69,   57,   64,   60,   30,   54,
      65,   75,   68,   64,   54,   76,   72,   67,
      72,   89,   60,   60,   75,   74,   98,   48,
      47,   59,   56,   17,   50,   48,   34,   32
  },
  {  // rook
      13,   52,   -2,   31,   44,   98,  121,   53,
       7,   -9,   20,   91,   85,   74,  -17,   14,
     -39,  -11,  -32,   47,   29,   11,    0,  -16,
     -42,  -47,  -28,  -22,  -19,   -4,  -29,  -49,
     -59,  -96,  -65,  -66,  -64,  -42,  -66,  -44,
     -70,  -66,  -73,  -71,  -41,  -54,  -35,  -62,
     -60,  -63,  -64,  -49,  -35,  -52,  -21,  -68,
     -37,  -31,  -27,   -7,   -4,  -16,  -38,  -18
  },
  {  // queen
     706,  694,  740,  746,  802,  805,  743,  769,
     720,  696,  721,  703,  721,  787,  782,  744,
     746,  727,  748,  760,  797,  787,  779,  756,
     720,  726,  736,  730,  728,  750,  712,  775,
     743,  709,  730,  721,  741,  729,  736,  748,
     708,  748,  727,  739,  728,  744,  756,  730,
     749,  744,  770,  758,  768,  773,  792,  750,
     732,  740,  741,  758,  750,  718,  770,  759
  },
  {  // king
      45,  311, -202, -189,   15,  629,  787,  888,
     112,  221, -194, -505, -207,  414,  585, 1152,
      31, -454, -668, -443, -149,  -27,  409,  471,
    -129, -310, -362, -287, -145,  -76,    7, -106,
    -198, -129, -105, -164, -153, -110, -122, -166,
    -112,  -65, -104, -120, -125, -107,  -80,  -87,
       5,  -41,  -82,  -93,  -91,  -70,  -26,  -37,
    -167,   -6,  -18,  -94,  -48,  -68,    8,   -9
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     159,  218,  196,  154,  152,  219,  259,  195,
      86,   81,   67,   25,   30,   53,   90,   68,
      63,   40,   35,   21,   30,   39,   35,   51,
      53,   42,   21,   29,   31,   34,   45,   45,
      46,   33,   29,   28,   42,   56,   25,   38,
      50,   37,   35,   47,   61,   59,   39,   43,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -103,   12,    8,  -23,  -42,   20,  -63, -123,
     -67,  -27,  -51,   51,   19,  -28,  -21,  -18,
     -62,  -39,   34,   -7,  -11,    0,  -23,  -57,
     -29,  -11,   -6,    2,   29,    6,   25,  -60,
     -45,  -15,   -2,    7,    7,  -15,  -22,  -67,
     -75,  -38,  -20,  -22,  -12,  -28,  -40,  -82,
     -94,  -96,  -54,  -44,  -51,  -73,  -86,  -98,
    -105, -118,  -82,  -53,  -91,  -71, -104, -112
  },
  {  // bishop
     -23,   11,   44,   29,   26,   43,   24,  -28,
     -25,   -7,   -2,   -1,   55,  -11,   -6,  -44,
      38,   -7,    4,    4,  -16,   74,  -13,   18,
     -23,    3,   -2,   -7,  -12,   -6,   -4,  -11,
     -35,   -2,   -7,   -8,  -16,    1,  -15,  -32,
     -32,  -28,   -7,    3,    6,  -24,  -17,  -14,
     -80,  -46,  -42,  -38,  -34,  -44,  -27,  -31,
     -82,  -76,  -60,  -36,  -51,  -37,  -45,  -63
  },
  {  // rook
      77,   64,   84,   72,   63,   35,   50,   75,
      98,  100,  108,   79,   70,   64,   91,   88,
      97,   92,  109,   67,   72,   84,   89,   85,
      85,   84,   90,   78,   76,   67,   69,   83,
      72,   93,   90,   81,   80,   66,   72,   57,
      70,   63,   71,   58,   45,   48,   42,   49,
      30,   37,   38,   42,   26,   24,   23,   30,
      40,   43,   62,   41,   43,   48,   50,   13
  },
  {  // queen
     116,  175,  134,  169,  121,   81,   97,   51,
      99,  189,  186,  245,  245,  128,  111,  110,
      59,  140,  164,  184,  145,  147,  115,   74,
     109,  129,  174,  187,  218,  178,  185,   56,
      78,  127,  144,  175,  165,  133,   96,   37,
      70,   35,  106,   64,   83,   81,   16,   38,
     -52,  -36,  -32,  -26,  -15,  -62, -159, -144,
      23,  -47,  -20,  -53,  -45,  -31,  -84,  -29
  },
  {  // king
     -33, -185,  -83,  116,   33, -120, -297, -291,
      27,   85,  176,  268,  186,    7, -133, -319,
      69,  316,  352,  290,  189,  160,    1, -122,
      99,  239,  265,  226,  194,  162,  113,   94,
     131,  148,  154,  183,  178,  152,  142,  114,
      89,  108,  137,  153,  158,  143,  115,   90,
      66,  102,  120,  139,  129,  121,   98,   69,
     127,   52,   83,   90,   82,   74,   68,   28
  }
};

// These eight live in src/evaluation.cpp, not eval_tables.hpp.
const int mobility_mg[4] = {0, 6, 9, 4};  // knight bishop rook queen
const int mobility_eg[4] = {-1, 4, -2, -9};  // knight bishop rook queen

const int king_safety_mg[KS_FEATURE_COUNT] = {
    22,  // KS_KNIGHT_ATTACKERS
    23,  // KS_BISHOP_ATTACKERS
    25,  // KS_ROOK_ATTACKERS
    35,  // KS_QUEEN_ATTACKERS
    -30,  // KS_ZONE_ATTACKS
    26,  // KS_SHIELD_NEAR
    13,  // KS_SHIELD_FAR
    -42,  // KS_OPEN_FILE
    -27,  // KS_HALF_OPEN_FILE
};

const int king_safety_eg[KS_FEATURE_COUNT] = {
    -11,  // KS_KNIGHT_ATTACKERS
    -15,  // KS_BISHOP_ATTACKERS
    -13,  // KS_ROOK_ATTACKERS
    -57,  // KS_QUEEN_ATTACKERS
    11,  // KS_ZONE_ATTACKS
    -11,  // KS_SHIELD_NEAR
    -11,  // KS_SHIELD_FAR
    -12,  // KS_OPEN_FILE
    26,  // KS_HALF_OPEN_FILE
};

const int passed_pawn_mg[6] = {-2, -6, -1, 22, 61, -1};  // second rank .. seventh

const int passed_pawn_eg[6] = {19, 22, 46, 69, 93, 25};  // second rank .. seventh

const int pawn_structure_mg[3] = {-9, -10, -12};  // isolated doubled backward

const int pawn_structure_eg[3] = {-13, -27, -8};  // isolated doubled backward

const int piece_placement_mg[4] = {0, 0, 0, 0};  // pair, open, half open, seventh

const int piece_placement_eg[4] = {0, 0, 0, 0};  // pair, open, half open, seventh

const int tempo_mg = 0;
const int tempo_eg = 0;
