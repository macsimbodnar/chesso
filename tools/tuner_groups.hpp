#pragma once
// The tuner's `--only` groups and the parameter range each one frees. S041.
//
// Split out of tools/tuner.cpp so a test can reach it. The function below has
// swallowed the term appended after it four times -- its own comments record
// all four -- and nothing in tests/ linked tuner.cpp at all
// (2026-08-13_adversarial-F07). tests/test_tuner_groups.cpp now holds the three
// properties the ranges have to have: non-empty, pairwise disjoint, and a union
// of exactly [0, PARAM_COUNT).
//
// A header rather than a second translation unit because free_mask() has no
// state and needs nothing but the layout constants, in tools/tuner_layout.hpp
// since S262 (tools/eval_model.hpp until then). The
// using-declarations below are what let the function body stay byte-identical
// to the one that lived in tuner.cpp; they are visible as tuner_groups::MG_BASE
// and so on, which is a small price for a move that changed no line of the
// body.
//
// Since S275 it also holds the seven gauges, the null directions a fit keeps
// still, because whether one is held depends on the same mask.
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include "tuner_layout.hpp"

namespace tuner_groups
{

using tuner_layout::EG_BASE;
using tuner_layout::KS_MG_BASE;
using tuner_layout::MATERIAL_COUNT;
using tuner_layout::MG_BASE;
using tuner_layout::MOB_MG_BASE;
using tuner_layout::PARAM_COUNT;
using tuner_layout::PIECE_PLACEMENT_COUNT;
using tuner_layout::PL_EG_BASE;
using tuner_layout::PL_MG_BASE;
using tuner_layout::PP_MG_BASE;
using tuner_layout::PS_MG_BASE;
using tuner_layout::TEMPO_MG_BASE;


inline constexpr const char* GROUP_LIST =
    "all, material, psqt, mobility, king_safety, passed_pawns, "
    "pawn_structure, piece_placement, tempo, passed_advance, passed_kings, "
    "candidate_passers, isolated_backward, isolated_files, doubled_files, "
    "backward_ranks, phalanx, supported, weak_unopposed";


// The piece placement group's terms, S135, each a name `--only` and `--freeze`
// take beside the groups above. DEC-259's screen fits every term of a family
// alone with every other constant frozen, and then the terms that passed
// together: `--only piece_placement.bad_bishop` for one term, and `--only
// piece_placement --freeze piece_placement.trapped` for the group without
// one. A term is one or more adjacent columns of the group, middlegame and
// endgame both, so the two bad bishop counts are screened as one term and the
// two trapped piece counts as another.
//
// Not in GROUP_LIST, on purpose: those partition the vector and
// tests/test_tuner_groups.cpp holds them to it, and a term is a part of one of
// them, so it overlaps the group it belongs to by construction. The same file
// holds the terms to their own partition of the group instead.
//
// The columns are numbered by hand. The engine's own enum is inside
// src/evaluation.cpp, where no tool can include it; its order is stated in
// prose over piece_placement_mg in src/evaluation.hpp, and the assert below
// ties the widths to the engine's arrays through tools/tuner_layout.hpp.
struct placement_term_t
{
  const char* name;
  size_t first;  // the term's first column within the group
  size_t width;  // how many adjacent columns it holds
};

inline constexpr placement_term_t PLACEMENT_TERMS[] = {
    {"piece_placement.bishop_pair", 0, 1},
    {"piece_placement.rook_open", 1, 1},
    {"piece_placement.rook_half_open", 2, 1},
    {"piece_placement.bad_bishop", 3, 2},
    {"piece_placement.trapped", 5, 2},
};

inline constexpr size_t PLACEMENT_TERM_COUNT =
    sizeof(PLACEMENT_TERMS) / sizeof(PLACEMENT_TERMS[0]);

static_assert(PLACEMENT_TERMS[PLACEMENT_TERM_COUNT - 1].first +
                      PLACEMENT_TERMS[PLACEMENT_TERM_COUNT - 1].width ==
                  PIECE_PLACEMENT_COUNT,
              "the engine's piece placement arrays changed width: name the "
              "terms here again");

inline constexpr const char* PLACEMENT_TERM_LIST =
    "piece_placement.bishop_pair, piece_placement.rook_open, "
    "piece_placement.rook_half_open, piece_placement.bad_bishop, "
    "piece_placement.trapped";


// Marks the parameters a --only group leaves free; the rest keep the value the
// engine ships and come out of the run unchanged.
//
// This exists for attribution under SPRT. Fitting a new term jointly also
// refits the 781 constants that were already fitted, so the match that follows
// measures two changes at once and its number says nothing about either. One
// change at a time; DEC-020 is what that rule cost to learn.
//
// Each group is one contiguous range because tools/tuner_layout.hpp lays the
// vector out that way.
inline bool free_mask(const std::string& group, std::vector<uint8_t>* mask)
{
  // A piece placement term, S135: its columns in both halves of the group,
  // which are two ranges and not one, so it is handled before the single-range
  // groups below.
  for (const placement_term_t& term : PLACEMENT_TERMS) {
    if (group != term.name) { continue; }

    mask->assign(PARAM_COUNT, 0);

    for (size_t i = 0; i < term.width; ++i) {
      (*mask)[PL_MG_BASE + term.first + i] = 1;
      (*mask)[PL_EG_BASE + term.first + i] = 1;
    }

    return true;
  }

  size_t first = 0;
  size_t last = 0;

  if (group == "all") {
    first = 0;
    last = PARAM_COUNT;
  } else if (group == "material") {
    first = 0;
    last = MATERIAL_COUNT;
  } else if (group == "psqt") {
    first = MG_BASE;
    last = MOB_MG_BASE;
  } else if (group == "mobility") {
    first = MOB_MG_BASE;
    last = KS_MG_BASE;
  } else if (group == "king_safety") {
    // Ends at the passed pawn block, not at PARAM_COUNT. A group that reaches
    // to the end of the vector silently swallows whatever term is appended
    // after it, and the run that follows measures two changes at once.
    first = KS_MG_BASE;
    last = PP_MG_BASE;
  } else if (group == "passed_pawns") {
    // Ends at the pawn structure block for the reason above, and it reached to
    // PARAM_COUNT until that block was appended -- the same bug the comment
    // over king_safety describes, one term later.
    first = PP_MG_BASE;
    last = PS_MG_BASE;
  } else if (group == "pawn_structure") {
    // Ends at the piece placement block for the reason above, and it reached to
    // PARAM_COUNT until that block was appended -- the third time this same
    // function has swallowed the term appended after it.
    first = PS_MG_BASE;
    last = PL_MG_BASE;
  } else if (group == "piece_placement") {
    // Ends at the tempo block for the reason above, and it reached to
    // PARAM_COUNT until that block was appended -- the fourth term in a row to
    // meet the same defect in the same function.
    first = PL_MG_BASE;
    last = TEMPO_MG_BASE;
  } else if (group == "tempo") {
    // Ends at its own block since S123. It ran to PARAM_COUNT until then and
    // would have swallowed S123's family appended after it -- the fifth time
    // this function met the defect. Every group now ends at its block's end
    // constant in tools/tuner_layout.hpp, and tests/test_tuner_groups.cpp
    // holds each to exactly its block, so the last one is no exception.
    first = TEMPO_MG_BASE;
    last = tuner_layout::TEMPO_END;
  } else if (group == "passed_advance") {
    // S123's A: the free and safe increments, both halves.
    first = tuner_layout::PF_MG_BASE;
    last = tuner_layout::PASSER_ADVANCE_END;
  } else if (group == "passed_kings") {
    // S123's B: both kings' distance slopes, both halves.
    first = tuner_layout::PKO_MG_BASE;
    last = tuner_layout::PASSER_KINGS_END;
  } else if (group == "candidate_passers") {
    // S123's C.
    first = tuner_layout::CP_MG_BASE;
    last = tuner_layout::CANDIDATE_PASSER_END;
  } else if (group == "isolated_backward") {
    // S125's seven, one block each (DEC-331 (4)): P1, isolated pawns with
    // their stop attacked. Screened as an increment over isolated and backward;
    // since S125's close the pawns' own weight, and backward's flat holds none
    // of them (DEC-331 (3)).
    first = tuner_layout::IB_MG_BASE;
    last = tuner_layout::ISOLATED_BACKWARD_END;
  } else if (group == "isolated_files") {
    // P2, isolated pawns by file.
    first = tuner_layout::ISF_MG_BASE;
    last = tuner_layout::ISOLATED_FILE_END;
  } else if (group == "doubled_files") {
    // P3, doubled pawns by file.
    first = tuner_layout::DBF_MG_BASE;
    last = tuner_layout::DOUBLED_FILE_END;
  } else if (group == "backward_ranks") {
    // P4, backward pawns not isolated by rank.
    first = tuner_layout::BKR_MG_BASE;
    last = tuner_layout::BACKWARD_RANK_END;
  } else if (group == "phalanx") {
    // P5.
    first = tuner_layout::PHX_MG_BASE;
    last = tuner_layout::PHALANX_END;
  } else if (group == "supported") {
    // P6.
    first = tuner_layout::SUP_MG_BASE;
    last = tuner_layout::SUPPORTED_END;
  } else if (group == "weak_unopposed") {
    // P7.
    first = tuner_layout::WKU_MG_BASE;
    last = tuner_layout::WEAK_UNOPPOSED_END;
  } else {
    return false;
  }

  mask->assign(PARAM_COUNT, 0);

  for (size_t i = first; i < last; ++i) {
    (*mask)[i] = 1;
  }

  return true;
}


// Holds the named groups at what the engine ships and leaves everything else as
// the mask found it. The inverse of --only, which frees one group: this frees
// all of them but a few. S065 needed "everything except tempo and
// piece_placement" and --only could only say "tempo" or "piece_placement".
//
// It does not touch the partition above -- it selects among the same ranges
// free_mask already defines, so test_tuner_groups' three properties are
// unaffected by anything here. A name free_mask refuses is refused here too,
// and an empty list is a no-op, which is what keeps the default behaviour of a
// run that passes no --freeze byte-identical.
//
// Why freeze during the fit rather than zero two groups afterwards: with the
// two groups held, the remaining 815 absorb what they would have taken (817 at
// S065, before S134 narrowed the vector to 823; still 815 since S135 widened
// it to 831, the four columns it added being piece placement's). Fitting all
// of them and then zeroing the two leaves those 815 fitted against values
// that are no longer there. DEC-057.
inline bool freeze_mask(const std::string& groups, std::vector<uint8_t>* mask)
{
  if (mask->size() != PARAM_COUNT) { return false; }

  std::string name;
  std::vector<uint8_t> group_mask;

  // One pass over the list, closing each name at a comma and at the end.
  for (size_t i = 0; i <= groups.size(); ++i) {
    const char c = (i < groups.size()) ? groups[i] : ',';

    if (c == ' ') { continue; }

    if (c != ',') {
      name.push_back(c);
      continue;
    }

    if (name.empty()) { continue; }

    // `all` is accepted by free_mask and would freeze the whole vector, which
    // is a run that fits nothing and reports the starting error. Refused here
    // rather than obeyed, because nothing wants it and a typo that produces it
    // is silent.
    if (name == "all") { return false; }

    if (!free_mask(name, &group_mask)) { return false; }

    for (size_t j = 0; j < PARAM_COUNT; ++j) {
      if (group_mask[j]) { (*mask)[j] = 0; }
    }

    name.clear();
  }

  return true;
}


// `--only` with a list, S123: the union of the named groups and terms, each
// as free_mask() frees it. S123's screen frees a part's own weights with the
// earlier survivors' -- `--only passed_advance,passed_kings` -- and one name
// is the list of one, so every run before it frees exactly what it did. A
// name free_mask() refuses refuses the list, and so does `all` beside
// another name, which would be the whole vector whatever the rest said.
inline bool only_mask(const std::string& groups, std::vector<uint8_t>* mask)
{
  if (groups.find(',') == std::string::npos) { return free_mask(groups, mask); }

  mask->assign(PARAM_COUNT, 0);

  std::string name;
  std::vector<uint8_t> group_mask;
  size_t names = 0;

  for (size_t i = 0; i <= groups.size(); ++i) {
    const char c = (i < groups.size()) ? groups[i] : ',';

    if (c == ' ') { continue; }

    if (c != ',') {
      name.push_back(c);
      continue;
    }

    if (name.empty()) { return false; }
    if (name == "all") { return false; }
    if (!free_mask(name, &group_mask)) { return false; }

    for (size_t j = 0; j < PARAM_COUNT; ++j) {
      if (group_mask[j]) { (*mask)[j] = 1; }
    }

    names++;
    name.clear();
  }

  return names > 0;
}


// The evaluation's exact null directions, and where a fit holds them. S275.
//
// tools/feature_audit's identity report puts the design at rank 784 of 823
// (S134). Thirty-two of the 39 null directions are single pawn columns on ranks
// 1 and 8, which no row can touch -- load() refuses a pawn there -- and Adam
// leaves those exactly where they started: a gradient of exactly zero keeps the
// moment and the velocity at zero, and the step is 0 / (0 + epsilon). The other
// seven are spread over many columns, and there Adam's per-coordinate scaling
// does not keep a step orthogonal to them, so a fit walks along them for
// nothing: no evaluation moves, but up to 129 shipped numbers do, and two fits'
// tables stop being comparable. These are the seven:
//
//   material/table, one per piece but the king: piece_value[t] + c and every
//     square of psqt_mg[t] and psqt_eg[t] a piece of type t can stand on - c.
//     The taper's two weights sum to one, so no piece's score moves.
//   psqt_mg[king] and psqt_eg[king]: every square + c. One king a side, so the
//     White king's + c and the Black king's - c cancel before the taper; load()
//     refuses a row without one king a side, as load_FEN() does.
//
// The gauge picks one point on each of those lines: the level -- the sum over
// the squares its direction moves, psqt_mg[t] and psqt_eg[t] together for a
// piece, one king table's own sum for the king -- stays at the value the fit
// started from, and a piece's value carries the shift. With material and psqt
// both free, a fit's tables then compare square by square with the ones it
// started from, and a change in what a piece is worth shows up in its define,
// where a reader looks for it. Under --only psqt or --freeze material only the
// two king sums are held: the values are frozen, so the tables carry any change
// in a piece's worth, as they always did.
inline constexpr size_t KING_TYPE = 5;

struct gauge_t
{
  std::string name;
  std::vector<size_t> level;  // the columns whose sum is held
  size_t absorb =
      PARAM_COUNT;  // the column the shift goes to; PARAM_COUNT: none
};


// The model's square, mirrored for Black, so one test covers both colours: 0..7
// is a pawn's own eighth rank and 56..63 its own first.
inline bool pawn_can_stand(size_t square)
{ return square >= 8 && square < 56; }


// The seven, in the order feature_audit reports them. The pawn's level leaves
// out the 32 columns no pawn can reach, so a hold never moves them either.
inline std::vector<gauge_t> gauges()
{
  static const char* const names[KING_TYPE + 1] = {"pawn", "knight", "bishop",
                                                   "rook", "queen",  "king"};
  const size_t bases[2] = {MG_BASE, EG_BASE};

  std::vector<gauge_t> out;

  for (size_t type = 0; type < MATERIAL_COUNT; ++type) {
    gauge_t gauge;
    gauge.name = std::string("material/table ") + names[type];
    gauge.absorb = type;

    for (const size_t base : bases) {
      for (size_t square = 0; square < 64; ++square) {
        if (type == 0 && !pawn_can_stand(square)) { continue; }
        gauge.level.push_back(base + type * 64 + square);
      }
    }

    out.push_back(gauge);
  }

  for (const size_t base : bases) {
    gauge_t gauge;
    gauge.name = (base == MG_BASE) ? "psqt_mg[king] sum" : "psqt_eg[king] sum";

    for (size_t square = 0; square < 64; ++square) {
      gauge.level.push_back(base + KING_TYPE * 64 + square);
    }

    out.push_back(gauge);
  }

  return out;
}


// The gauges a run holds: those whose every column the mask leaves free. A
// direction with a frozen column needs no hold -- the free parameters cannot
// move along it without moving that column -- and holding it anyway would write
// a parameter --only or --freeze promised to leave bit-identical.
inline std::vector<gauge_t> held_gauges(const std::vector<uint8_t>& mask)
{
  std::vector<gauge_t> out;

  for (const gauge_t& gauge : gauges()) {
    bool free = (gauge.absorb == PARAM_COUNT) || mask[gauge.absorb] != 0;

    for (const size_t column : gauge.level) {
      free = free && mask[column] != 0;
    }

    if (free) { out.push_back(gauge); }
  }

  return out;
}


// Puts each held gauge's level back where `start` had it and moves the
// difference into the piece's value: c off every column of the level and onto
// the absorbing one, a move along the null direction and so invisible to every
// evaluation. Called after each step, so no vector a fit reports or emits is
// ever off the gauge.
inline void hold_gauges(const std::vector<gauge_t>& held,
                        const double* start,
                        double* params)
{
  for (const gauge_t& gauge : held) {
    double drift = 0.0;

    for (const size_t column : gauge.level) {
      drift += params[column] - start[column];
    }

    drift /= static_cast<double>(gauge.level.size());

    for (const size_t column : gauge.level) {
      params[column] -= drift;
    }

    if (gauge.absorb < PARAM_COUNT) { params[gauge.absorb] += drift; }
  }
}


// Tied mobility cells, S121. A cell the corpus barely reaches is not fitted
// on its own -- AdaGrad and Adam step a coordinate at the learning rate however
// few rows its gradient comes from, so a cell a handful of rows reach walks to
// whatever those rows' results say -- and it is not left at its seed either,
// because DEC-084 ships no constant unfitted. It is tied: it shares one
// parameter with a supported cell of the same piece and half, and ships that
// cell's fitted value times the tie's scale. The tie list is a run's input,
// written before the fit from tools/feature_audit's support report
// (adocs/data/S121_screen.py ties), and stamped into the emitted header.
//
// The same map serves the linear model DEC-326 screens a part over when the
// curve itself is dropped: every cell of a row tied to its count-1 cell with
// scale c, so the row is c times one weight -- the four-weight term S121
// replaced, over whatever count the part defines.
//
// One line a tie, `#` to the end of a line a comment:
//
//   <row> <cell> <anchor> [<scale>]
//
// row mobility_<piece>_<half>, piece knight, bishop, rook or queen, or since
// S123 one of its passed pawn arrays, passed_free, passed_safe,
// passed_king_own, passed_king_enemy or candidate_passer, followed by _<half>,
// whose cells are the own ranks from the second (DEC-329 (4): a weight under
// its support is tied to its nearest rank's same term), or since S125 one of
// its arrays, isolated_backward, isolated_file, doubled_file, backward_rank,
// phalanx, supported or weak_unopposed, followed by _<half>, whose cells are
// its weights' (src/evaluation.hpp; DEC-331 (6): the nearest supported cell of
// the same array and half); half mg or eg; scale 1 when absent. A cell is tied
// once at most, never to itself, and an anchor is never tied, so no tie reads a
// value another tie has yet to write.
struct tie_t
{
  size_t tied;
  size_t anchor;
  double scale;
};


// A tie row's name to its first parameter and its width; false for a name
// that is no row.
inline bool tie_row(const std::string& name, size_t* base, size_t* width)
{
  static const char* const pieces[4] = {"knight", "bishop", "rook", "queen"};

  for (int p = 0; p < 4; ++p) {
    for (const char* half : {"mg", "eg"}) {
      if (name == std::string("mobility_") + pieces[p] + "_" + half) {
        *base = ((half[0] == 'm') ? tuner_layout::MOB_MG_BASE
                                  : tuner_layout::MOB_EG_BASE) +
                static_cast<size_t>(MOBILITY_OFFSET[p]);
        *width = static_cast<size_t>(MOBILITY_WIDTH[p]);
        return true;
      }
    }
  }

  struct family_row_t
  {
    const char* name;
    size_t mg_base;
    size_t eg_base;
    size_t width;
  };

  static const family_row_t rows[] = {
      {"passed_free", tuner_layout::PF_MG_BASE, tuner_layout::PF_EG_BASE,
       tuner_layout::PASSED_FREE_COUNT},
      {"passed_safe", tuner_layout::PSF_MG_BASE, tuner_layout::PSF_EG_BASE,
       tuner_layout::PASSED_SAFE_COUNT},
      {"passed_king_own", tuner_layout::PKO_MG_BASE, tuner_layout::PKO_EG_BASE,
       tuner_layout::PASSED_KING_COUNT},
      {"passed_king_enemy", tuner_layout::PKE_MG_BASE,
       tuner_layout::PKE_EG_BASE, tuner_layout::PASSED_KING_COUNT},
      {"candidate_passer", tuner_layout::CP_MG_BASE, tuner_layout::CP_EG_BASE,
       tuner_layout::CANDIDATE_PASSER_COUNT},
      {"isolated_backward", tuner_layout::IB_MG_BASE, tuner_layout::IB_EG_BASE,
       tuner_layout::ISOLATED_BACKWARD_COUNT},
      {"isolated_file", tuner_layout::ISF_MG_BASE, tuner_layout::ISF_EG_BASE,
       tuner_layout::ISOLATED_FILE_COUNT},
      {"doubled_file", tuner_layout::DBF_MG_BASE, tuner_layout::DBF_EG_BASE,
       tuner_layout::DOUBLED_FILE_COUNT},
      {"backward_rank", tuner_layout::BKR_MG_BASE, tuner_layout::BKR_EG_BASE,
       tuner_layout::BACKWARD_RANK_COUNT},
      {"phalanx", tuner_layout::PHX_MG_BASE, tuner_layout::PHX_EG_BASE,
       tuner_layout::PHALANX_COUNT},
      {"supported", tuner_layout::SUP_MG_BASE, tuner_layout::SUP_EG_BASE,
       tuner_layout::SUPPORTED_COUNT},
      {"weak_unopposed", tuner_layout::WKU_MG_BASE, tuner_layout::WKU_EG_BASE,
       tuner_layout::WEAK_UNOPPOSED_COUNT},
  };

  for (const family_row_t& row : rows) {
    for (const char* half : {"mg", "eg"}) {
      if (name == std::string(row.name) + "_" + half) {
        *base = (half[0] == 'm') ? row.mg_base : row.eg_base;
        *width = row.width;
        return true;
      }
    }
  }

  return false;
}


inline bool parse_ties(const std::string& text,
                       std::vector<tie_t>* out,
                       std::string* error)
{
  out->clear();
  std::vector<uint8_t> tied(PARAM_COUNT, 0);
  std::vector<uint8_t> anchors(PARAM_COUNT, 0);
  size_t line_number = 0;
  size_t start = 0;

  while (start < text.size()) {
    size_t end = text.find('\n', start);
    if (end == std::string::npos) { end = text.size(); }

    std::string line = text.substr(start, end - start);
    start = end + 1;
    line_number++;

    const size_t hash = line.find('#');
    if (hash != std::string::npos) { line.resize(hash); }

    std::vector<std::string> words;
    std::string word;
    for (const char c : line + " ") {
      if (c == ' ' || c == '\t' || c == '\r') {
        if (!word.empty()) { words.push_back(word); }
        word.clear();
      } else {
        word.push_back(c);
      }
    }

    if (words.empty()) { continue; }

    const auto refuse = [&](const std::string& why) {
      *error = "tie line " + std::to_string(line_number) + ": " + why;
      return false;
    };

    if (words.size() != 3 && words.size() != 4) {
      return refuse("expected `<row> <cell> <anchor> [<scale>]`");
    }

    size_t base = PARAM_COUNT;
    size_t width = 0;

    if (!tie_row(words[0], &base, &width)) {
      return refuse("no table named " + words[0]);
    }

    const auto cell_of = [&](const std::string& s, size_t* cell) {
      if (s.empty() || s.size() > 2 ||
          s.find_first_not_of("0123456789") != std::string::npos) {
        return false;
      }
      const unsigned long value = std::stoul(s);
      if (value >= width) { return false; }
      *cell = static_cast<size_t>(value);
      return true;
    };

    size_t cell = 0;
    size_t anchor = 0;
    if (!cell_of(words[1], &cell) || !cell_of(words[2], &anchor)) {
      return refuse("a cell outside " + words[0] + "'s row");
    }

    if (cell == anchor) { return refuse("a cell tied to itself"); }

    double scale = 1.0;
    if (words.size() == 4) {
      char* rest = nullptr;
      scale = std::strtod(words[3].c_str(), &rest);
      if (rest == words[3].c_str() || *rest != '\0' || !std::isfinite(scale)) {
        return refuse("an unreadable scale " + words[3]);
      }
    }

    const tie_t tie{base + cell, base + anchor, scale};

    if (tied[tie.tied]) { return refuse("a cell tied twice"); }
    if (tied[tie.anchor] || anchors[tie.tied]) {
      return refuse(
          "a chain: an anchor that is tied, or a tied cell that "
          "anchors another");
    }

    tied[tie.tied] = 1;
    anchors[tie.anchor] = 1;
    out->push_back(tie);
  }

  return true;
}


// A tie binds only where the fit is free to move both ends: a frozen anchor
// would hand a frozen value to a cell the fit means to fit, and a frozen tied
// cell would be written by a run that promised to leave it alone.
inline bool ties_fit_mask(const std::vector<tie_t>& ties,
                          const std::vector<uint8_t>& mask,
                          std::string* error)
{
  for (const tie_t& tie : ties) {
    if (!mask[tie.tied] || !mask[tie.anchor]) {
      *error = "a tie whose cell or anchor the run holds frozen";
      return false;
    }
  }

  return true;
}


// The step's half of a tie: the tied cell takes no step of its own -- its
// parameter is the anchor's -- and its gradient joins the anchor's through the
// chain rule, d(error)/d(anchor) += scale * d(error)/d(tied).
inline void fold_ties(const std::vector<tie_t>& ties, std::vector<double>* grad)
{
  for (const tie_t& tie : ties) {
    (*grad)[tie.anchor] += tie.scale * (*grad)[tie.tied];
    (*grad)[tie.tied] = 0.0;
  }
}


// And the value's half: every tied cell is its anchor times the scale. The
// tuner calls it once before the first step -- after the run's start figures
// are read at the shipped constants, so a fit's gain is still read against
// what ships (DEC-315), and so the first gradient is taken on the tied
// parameterisation -- and after every step.
inline void copy_ties(const std::vector<tie_t>& ties, double* params)
{
  for (const tie_t& tie : ties) {
    params[tie.tied] = tie.scale * params[tie.anchor];
  }
}


// S121's eight mobility levels, the null directions of a table whose every
// piece adds one cell: per piece type and half, c on every cell of the row and
// -c on every square of the piece's table half moves no row the clamp does
// not bind (tools/feature_audit lists them). No gauge holds them -- a hold
// would move the clamped rows -- so a free mask that frees both sides of one
// is a fit that walks it for nothing but the clamped rows, S275's drift in
// another form. False, with `error` naming the first, for such a mask; true
// when every level has a frozen side, which is every `--only mobility` and
// `--only psqt` fit. A fit that frees both (S126's joint refit) must first
// give the levels a hold of their own.
inline bool mobility_levels_held(const std::vector<uint8_t>& mask,
                                 std::string* error)
{
  static const char* const pieces[4] = {"knight", "bishop", "rook", "queen"};

  for (int type = 0; type < 4; ++type) {
    for (int half = 0; half < 2; ++half) {
      const size_t cells = ((half == 0) ? tuner_layout::MOB_MG_BASE
                                        : tuner_layout::MOB_EG_BASE) +
                           static_cast<size_t>(MOBILITY_OFFSET[type]);
      const size_t table = ((half == 0) ? MG_BASE : EG_BASE) +
                           static_cast<size_t>(type + 1) * 64;

      bool cells_free = false;
      for (int c = 0; c < MOBILITY_WIDTH[type]; ++c) {
        cells_free = cells_free || mask[cells + static_cast<size_t>(c)] != 0;
      }

      bool table_free = false;
      for (size_t square = 0; square < 64; ++square) {
        table_free = table_free || mask[table + square] != 0;
      }

      if (cells_free && table_free) {
        *error = std::string("the free set walks the mobility level of the ") +
                 pieces[type] + ((half == 0) ? " middlegame" : " endgame") +
                 " half: its row's cells and its piece-square table are both "
                 "free and no gauge holds the level; freeze one side";
        return false;
      }
    }
  }

  return true;
}

}  // namespace tuner_groups
