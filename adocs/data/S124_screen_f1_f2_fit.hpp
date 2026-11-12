// Fitted by tools/tuner from 6307294 self-play positions.
// engine     c496403, the commit this tuner was built from
// data       /home/max/ws/chesso/.tuning/s122/prepared/corpus.tsv
// corpus     sha256 04c596de726b5a2886df4e7e41c08fdd958529c669ba98ee69befd6a54676509
//            6307294 rows, 484308773 bytes; `sha256sum` over the same file prints the same
// K          1.3248
// error      0.080856 train, 0.079263 validation
// seed       1, lr 1.000, validation split 0.10
// model      evaluate()'s trace, 6307294 rows; 0 differ from evaluate() at the shipped weights
// optimiser  adam
// run        20000 epochs max, report every 100, patience 20 reports, 12 threads
//            the emitted vector is the best reported epoch, so --report and
//            --patience choose it as much as the data does
// split      fixed held-out set, S082: 1384182 rows, every corpus row trains
//            /home/max/ws/chesso/.tuning/s122/prepared/pool.tsv.qs.validation
//            sha256 faef5cd415abfb999c77f9754d0d1fbcd81fa572d04680b2b73e1bb9508936eb
// only       endgame_scale.f1,endgame_scale.f2
// freeze     (nothing)
// gauges     0 of 7 held at the levels this started from
// tie        (nothing)
// lambda     0.0000  target = lambda * sigma(K * score) + (1 - lambda) * result
// margin     275, the stage-two clamp this fit was read at
//
// Paste over the corresponding definitions. The piece defines and the
// two tables live in src/eval_tables.hpp; the mobility, king safety,
// passed pawn, pawn structure, piece placement, tempo, S123's
// passed pawn family, S125's pawn structure family, S101's threat
// family, S122's king safety family and S124's endgame family
// weights at the end live in src/evaluation.cpp, a different file
// and easy to miss.
// S027, S028, S034 and S035, DEC-015: measured by SPRT before any of
// it is kept.

#define PAWN    84
#define KNIGHT  371
#define BISHOP  352
#define ROOK    519
#define QUEEN   983

static constexpr int psqt_mg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     222,  158,  185,  171,  197,   39,   32,   82,
      12,   25,   43,   68,   74,   72,    8,   43,
     -20,   14,   11,   26,   31,   18,   25,  -24,
     -25,  -14,    9,   16,   21,    8,    0,  -23,
     -20,  -11,   -2,  -12,   -2,  -11,   32,  -10,
     -33,   -5,  -16,  -36,  -29,    3,   29,  -26,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -183,  -16, -130,  -36,   10, -225,   69,  -54,
     -33,  -10,   40,   62,   11,   72,  -61,   11,
      -7,   61,   43,  100,  108,  106,   75,   21,
      11,   23,   39,   54,   27,   70,   26,   43,
     -16,   -8,   25,   22,   38,   38,   39,   18,
     -25,    1,   11,   32,   34,   29,   31,  -12,
     -40,  -28,    3,   10,   14,   19,   10,   -8,
     -16,  -25,  -19,  -28,  -15,   -2,  -16, -113
  },
  {  // bishop
      51,  -60, -138, -109,  -57, -239,  -31,   89,
      24,   27,   21,   40,  -21,   58,   10,   30,
      17,   54,   51,   60,   75,   13,   80,   32,
      36,   55,   48,   80,   56,   46,   49,   38,
      67,   29,   54,   64,   62,   51,   37,   43,
      41,   71,   58,   51,   47,   67,   72,   50,
      56,   66,   58,   48,   59,   66,   80,   34,
      20,   41,   42,   17,   50,   36,   26,   34
  },
  {  // rook
      -6,   -2,  -48,  -15,  -18,    9,  -13,    6,
     -21,  -64,  -11,   28,   31,   12,  -70,   -8,
     -43,  -24,  -26,   10,   48,   16,    4,  -39,
     -64,  -60,  -43,  -39,  -19,  -20,  -63,  -60,
     -87,  -76,  -77,  -66,  -62,  -54,  -78,  -80,
     -85,  -77,  -71,  -77,  -50,  -54,  -69,  -79,
     -70,  -50,  -60,  -55,  -44,  -50,  -42, -118,
     -52,  -42,  -36,  -16,   -9,  -14,  -72,  -56
  },
  {  // queen
     341,  350,  370,  417,  401,  417,  424,  419,
     367,  334,  353,  357,  344,  402,  381,  426,
     363,  371,  366,  405,  415,  430,  407,  404,
     364,  369,  383,  371,  382,  385,  384,  405,
     394,  364,  378,  369,  389,  387,  381,  382,
     386,  400,  382,  388,  379,  403,  399,  389,
     395,  397,  420,  410,  420,  419,  431,  390,
     397,  380,  399,  413,  399,  354,  373,  417
  },
  {  // king
      86,  291,   -5,    6,   79,  243,  191,  661,
      89,   40,   58, -130,   87,  102,    6,  196,
      -6,    6,  -72, -127,  -59,   15,  -11, -219,
     -46,  -68,  -41,  -63,  -41,  -11,  -91, -173,
    -144,  -47,  -38,  -61, -107,  -50,  -54,  -99,
     -66,   13,  -51,  -47,  -44,  -37,    2,  -46,
      57,    7,    9,  -21,  -19,    3,   33,   24,
     -21,   70,   70,  -30,   47,    2,   59,   49
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     190,  229,  215,  170,  161,  223,  262,  224,
      74,   62,   43,    7,   -2,   24,   57,   48,
      54,   37,   27,    8,   11,   26,   27,   41,
      41,   33,   17,   13,   15,   19,   22,   27,
      29,   20,   20,   28,   30,   33,    6,   15,
      37,   25,   35,   47,   39,   32,   15,   16,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
     -91,  -35,    1,  -32,  -37,   26,  -48, -122,
     -70,  -46,  -25,  -10,   -8,  -19,  -19,  -49,
     -51,  -22,    5,  -17,  -24,  -35,  -22,  -40,
     -40,  -16,    5,    9,   29,   -6,    3,  -41,
     -37,  -23,    3,   14,    8,   -1,  -31,  -48,
     -67,  -29,  -18,   -3,  -16,  -29,  -43,  -64,
     -81,  -43,  -49,  -32,  -38,  -55,  -69,  -69,
    -138,  -99,  -44,  -44,  -35,  -68, -101,  -41
  },
  {  // bishop
       5,    3,   33,    6,   14,   59,   -6,  -24,
     -34,    2,   -8,   -9,   16,  -22,    0,  -44,
       3,  -12,   -7,   -6,  -15,   12,  -16,  -12,
     -24,   -3,   -4,  -16,   -7,   -1,   -8,    0,
     -43,   -3,   -8,  -12,  -10,  -12,   -8,  -20,
     -12,  -24,   -7,    2,    5,  -23,  -30,  -21,
     -42,  -33,  -22,  -21,  -23,  -28,  -28,  -45,
     -36,  -36,  -36,  -15,  -45,  -25,  -18,  -33
  },
  {  // rook
      95,  102,  125,  116,  110,   99,  115,  100,
     110,  131,  123,  105,  109,  106,  130,  106,
     105,  105,  110,   88,   73,   92,   97,  102,
     106,  106,  107,   96,   91,   94,  112,  100,
     104,  103,  110,  101,   93,   98,  104,   97,
     105,   95,   99,  102,   79,   88,   92,   91,
      84,   74,   77,   80,   75,   77,   74,   96,
      75,   74,   84,   68,   64,   67,   95,   59
  },
  {  // queen
     -68,  -63,  -63,  -97,  -68,  -81, -111, -127,
     -81,  -48,  -20,  -27,  -11,  -65,  -50,  -79,
     -75,  -44,  -43,  -59,  -75,  -49,  -38,  -40,
     -87,  -85,  -78,  -64,  -51,  -46,  -19,  -70,
    -156,  -80,  -95,  -46,  -75,  -55,  -67,  -55,
    -181, -176, -106, -123,  -91, -111, -107, -137,
    -181, -171, -200, -157, -170, -177, -234, -166,
    -183, -182, -179, -192, -175, -162, -215, -174
  },
  {  // king
     -28,   54,   91,  102,   71,   54,  110, -180,
      31,  115,  105,  165,   94,   89,  136,  -17,
      85,  112,  133,  131,  113,  117,  124,   95,
      60,  106,  109,  104,  100,  105,  117,   83,
      86,   81,   94,  105,  112,   94,   78,   63,
      41,   52,   81,   91,   88,   73,   50,   43,
       7,   39,   56,   64,   63,   50,   34,   14,
      -7,   -5,   10,   38,   -6,   16,    0,  -33
  }
};

// The mobility tables live in src/evaluation.cpp, not eval_tables.hpp.
constexpr int mobility_knight_mg[9] = {-9, 10, 15, 15, 17, 20, 22, 29, 42};
constexpr int mobility_knight_eg[9] = {-46, -15, 3, 7, 8, 9, 8, 0, -19};
constexpr int mobility_bishop_mg[14] = {11, 20, 29, 38, 44, 48, 52, 53, 59, 62, 68, 88, 70, 91};
constexpr int mobility_bishop_eg[14] = {-29, -17, -1, 8, 18, 26, 33, 33, 35, 32, 32, 21, 35, 7};
constexpr int mobility_rook_mg[15] = {16, 27, 35, 42, 47, 55, 63, 70, 75, 84, 88, 94, 97, 97, 99};
constexpr int mobility_rook_eg[15] = {-48, -8, 3, 9, 12, 13, 18, 17, 22, 26, 27, 30, 32, 33, 25};
constexpr int mobility_queen_mg[28] = {-13, -13, -12, -9, -7, -6, -2, -1, 3, 6, 7, 9, 8, 7, 10, 7, 8, 12, 19, 24, 46, 78, 92, 96, 39, 79, 79, 79};
constexpr int mobility_queen_eg[28] = {-57, 18, 11, 6, 38, 44, 49, 65, 76, 89, 89, 96, 102, 111, 109, 110, 119, 117, 109, 94, 76, 48, 25, 8, 42, -22, -22, -22};

constexpr int king_safety_mg[KS_FEATURE_COUNT] = {
    18,  // KS_KNIGHT_ATTACKERS
    13,  // KS_BISHOP_ATTACKERS
    5,  // KS_ROOK_ATTACKERS
    30,  // KS_QUEEN_ATTACKERS
    -27,  // KS_ZONE_ATTACKS
    16,  // KS_SHIELD_NEAR
    7,  // KS_SHIELD_FAR
    -47,  // KS_OPEN_FILE
    -25,  // KS_HALF_OPEN_FILE
};

constexpr int king_safety_eg[KS_FEATURE_COUNT] = {
    -5,  // KS_KNIGHT_ATTACKERS
    -9,  // KS_BISHOP_ATTACKERS
    7,  // KS_ROOK_ATTACKERS
    -52,  // KS_QUEEN_ATTACKERS
    10,  // KS_ZONE_ATTACKS
    -8,  // KS_SHIELD_NEAR
    -1,  // KS_SHIELD_FAR
    -4,  // KS_OPEN_FILE
    19,  // KS_HALF_OPEN_FILE
};

constexpr int passed_pawn_mg[5] = {9, -5, -2, 21, 38};  // second rank .. sixth

constexpr int passed_pawn_eg[5] = {18, 24, 51, 75, 119};  // second rank .. sixth

constexpr int pawn_structure_mg[3] = {1, -7, -6};  // isolated doubled backward

constexpr int pawn_structure_eg[3] = {-11, -20, 0};  // isolated doubled backward

constexpr int piece_placement_mg[7] = {-5, 23, 3, 0, 0, -40, -23};  // pair, open, half open, bad bishop, rammed, trapped minor, trapped rook

constexpr int piece_placement_eg[7] = {37, -4, 13, 0, 0, -68, -16};  // pair, open, half open, bad bishop, rammed, trapped minor, trapped rook

constexpr int tempo_mg = 0;
constexpr int tempo_eg = 0;

// S123's passed pawn family, by own rank from the second.
constexpr int passed_free_mg[6] = {2, 8, 2, 7, 28, 76};
constexpr int passed_free_eg[6] = {23, 3, 14, 13, 5, 6};
constexpr int passed_safe_mg[6] = {-1, 1, 2, 0, 10, 33};
constexpr int passed_safe_eg[6] = {8, 6, 10, 22, 51, 53};
constexpr int passed_king_own_mg[6] = {2, 5, 7, 3, -2, -3};
constexpr int passed_king_own_eg[6] = {-3, -5, -13, -16, -17, -19};
constexpr int passed_king_enemy_mg[6] = {-2, -7, -7, -5, 0, 2};
constexpr int passed_king_enemy_eg[6] = {-3, 3, 9, 15, 18, 23};
constexpr int candidate_passer_mg[5] = {-6, -16, 4, 10, -2};
constexpr int candidate_passer_eg[5] = {4, 8, 30, 46, 59};

// S125's pawn structure family, by the cells src/evaluation.hpp names.
constexpr int isolated_backward_mg[1] = {-6};
constexpr int isolated_backward_eg[1] = {0};
constexpr int isolated_file_mg[7] = {1, -6, -11, -10, -3, -7, 1};
constexpr int isolated_file_eg[7] = {-3, -1, -2, 0, -1, 3, 2};
constexpr int doubled_file_mg[7] = {0, 0, 0, 0, 0, 0, 0};
constexpr int doubled_file_eg[7] = {0, 0, 0, 0, 0, 0, 0};
constexpr int backward_rank_mg[3] = {0, 0, 0};
constexpr int backward_rank_eg[3] = {0, 0, 0};
constexpr int phalanx_mg[6] = {5, 2, 7, 20, 86, 86};
constexpr int phalanx_eg[6] = {-1, 1, 4, 7, 68, 68};
constexpr int supported_mg[5] = {8, 5, 7, 32, 187};
constexpr int supported_eg[5] = {11, 0, 4, 26, -29};
constexpr int weak_unopposed_mg[1] = {-10};
constexpr int weak_unopposed_eg[1] = {-6};

// S101's threat family, by the cells src/evaluation.hpp names.
constexpr int threat_pawn_mg[4] = {34, 32, 22, 20};
constexpr int threat_pawn_eg[4] = {10, 33, 33, 1};
constexpr int threat_piece_mg[5] = {14, 6, 13, 22, 44};
constexpr int threat_piece_eg[5] = {12, 31, 19, 62, 30};
constexpr int threat_minor_mg[2] = {14, 12};
constexpr int threat_minor_eg[2] = {20, 14};
constexpr int hanging_piece_mg[4] = {16, 12, 10, 15};
constexpr int hanging_piece_eg[4] = {10, 12, -7, -14};
constexpr int hanging_pawn_mg[1] = {1};
constexpr int hanging_pawn_eg[1] = {15};
constexpr int threat_king_mg[4] = {12, 48, 42, 62};
constexpr int threat_king_eg[4] = {20, 0, 13, -11};
constexpr int threat_push_mg[4] = {14, 15, 4, 7};
constexpr int threat_push_eg[4] = {13, 14, 10, 8};

// S122's king safety family, by the cells and inputs src/evaluation.hpp names.
constexpr int ks_checks_mg[4] = {-56, -11, -84, -14};
constexpr int ks_checks_eg[4] = {4, -14, 24, -4};
constexpr int ks_weak_mg[1] = {3};
constexpr int ks_weak_eg[1] = {4};
constexpr int ks_defenders_mg[1] = {0};
constexpr int ks_defenders_eg[1] = {0};
constexpr int ks_storm_mg[7] = {44, -15, -6, -1, -7, 5, -7};
constexpr int ks_storm_eg[7] = {7, 3, -1, -2, -11, -7, -2};
constexpr int ks_curve_mg[12] = {-96, 132, 111, 123, 175, -17, 55, 55, 30, 73, 73, 0};
constexpr int ks_curve_eg[12] = {229, -34, -13, -9, -512, 15, -39, -25, -1, 39, -5, 0};

// S124's endgame family: mop-up's counts and the factor's cells, as src/evaluation.hpp names them.
constexpr int mopup_mg[MOPUP_FEATURE_COUNT] = {0, 0, 0};  // centre, kings, corner
constexpr int mopup_eg[MOPUP_FEATURE_COUNT] = {0, 0, 0};  // centre, kings, corner
constexpr int endgame_scale[ENDGAME_CELL_COUNT] = {45, 0, 9, 29, 81, 114, 86, 128, 123, 19, 128, 128, 128};  // P0 d<=0 d1 d>=2, P1 .., P2 .., OCB pure P<=2 3-4 >=5, OCB rook
