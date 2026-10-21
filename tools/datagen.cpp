// Self-play position generator for evaluation tuning. S028; the leaf, S082.
//
// Plays chesso against itself at a fixed node budget from a randomised opening
// and writes, one per line:
//
//   fen  result  score  phase  game  ply  distance
//
// `result` is the game's outcome from White's point of view, 1 win, 0.5 draw,
// 0 loss. `score` is the engine's own score for the sampled position, White
// relative, and `phase` the game_phase() of the position the row's FEN names.
// The tuner reads those four. `game` numbers the game inside this run, `ply`
// is the sampled position's half-move, `2 * (fullmove - 1) + (black to move)`,
// and `distance` is how many plies past it the FEN stands.
// tools/tuner_split.hpp reads `ply` to find where one game ends, because the
// FEN's own move number stops advancing inside a game once the row is a leaf;
// tools/corpus_split.py reads `game` to build the fixed validation set.
//
// THE ROW NAMES A RESOLVED POSITION, S082. Which one is `--leaf`'s choice:
//
//   root  the position the game reached -- every corpus before S082
//   qs    the leaf quiescence resolves the sampled position to, a function of
//         its FEN alone (src/leaf_walk.hpp has the walk and its policies)
//   pv    the last completed iteration's principal variation played out, its
//         end resolved the same way
//
// Several at once write one file each, `--out` with the kind appended, from
// the same games and the same sampled positions: the pool the recipe is chosen
// from offline (adocs/data/S082_recipe.sh). A leaf the walk refuses -- a
// position the rules decide, a walk its bound truncated, a call that ran out
// of nodes or an answer it could not certify -- is not written, and counted.
//
// The label keeps one perspective whoever moves at the leaf: `result` and
// `score` are White's, and the tuner turns its White-relative model into the
// side to move's by the FEN's own side to move, so a leaf an odd number of
// plies on means what the root meant.
//
// Three independent filter clauses decide whether a sampled position is
// written at all, each read at the position the game stands on: the side to
// move is not in check, the search found no mate, and the score is inside
// --quiet-limit. Until S082 a fourth dropped a position whose best move was a
// capture or a promotion, on the claim that the root might not be quiet --
// `--allow-tactical` since DEC-055, which found quiescence evaluating exactly
// those positions. The row names a quiet position now whatever the root was,
// so the clause and its flag are retired rather than kept switched off.
//
// Every run prints what each clause cost and, for every leaf, how often it
// differs from the root and how often the walk was refused, so the next corpus
// is sized from a measurement rather than from this comment.
//
// Nothing here reads another engine. The positions are chesso's own play and
// the labels are game outcomes, which is what DEC-016 requires of training
// data.
#include <algorithm>
#include <array>
#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "correction_history.hpp"
#include "evaluation.hpp"
#include "leaf_walk.hpp"
#include "search.hpp"
#include "transposition_table.hpp"

namespace
{

// Every hardware thread the machine has. DEC-050 sets the policy at all of
// them, SMT included; asking the machine rather than writing the number keeps
// it true on the next one. hardware_concurrency() may answer 0 when it cannot
// tell.
unsigned default_threads()
{
  const unsigned reported = std::thread::hardware_concurrency();
  return (reported > 0) ? reported : 1;
}


// Which position a row names. S082.
enum leaf_kind_t : uint8_t
{
  KIND_ROOT = 0,
  KIND_QS,
  KIND_PV,
  KIND_COUNT
};

const char* const KIND_NAMES[KIND_COUNT] = {"root", "qs", "pv"};


struct options_t
{
  uint64_t games = 1000;
  uint64_t nodes = 5000;
  int random_plies = 8;
  int max_plies = 400;
  int opening_limit = 400;  // discard an opening already this lopsided
  int resign_score = 2000;  // adjudicate once one side is this far ahead
  int resign_plies = 6;     // ... for this many plies in a row
  int quiet_limit = 1000;   // do not record a position scored beyond this
  // The leaves written, in output order; the quiescence leaf unless --leaf
  // names others. S082.
  std::vector<leaf_kind_t> leaves = {KIND_QS};
  unsigned threads = default_threads();
  uint64_t seed = 1;
  int tt_mb = 16;
  std::string out = "";
};

struct sample_t
{
  std::string fen;
  int score;     // White relative, the sampled position's
  int phase;     // of the position `fen` names
  int ply;       // the sampled position's
  int distance;  // plies from the sampled position to the one `fen` names
};

// search() polls this and never sets it. Datagen has no clock, so it is only
// ever false; the node budget is what stops a search.
std::atomic_bool never_stop{false};

std::mutex output_lock;
std::atomic<uint64_t> games_done{0};

// Set by the first row or flush that does not reach its file. The counts the
// run prints are of rows it meant to write, so after a failed write they
// describe a corpus the files do not hold: every worker stops at its next
// game and main() exits 1. Until S082's phase 2 the failure was ignored and
// the run exited 0 (tests/test_datagen_exit.sh).
std::atomic_bool write_failed{false};

// What the filter threw away, counted per clause and printed at the end. The
// clauses are independent tests, not a chain: each counter below is the number
// of positions that would have been recorded but for that one clause, so it
// prices exactly what loosening that clause alone would admit. A position
// failing two clauses is in neither count. `considered` is every position a
// completed search returned a move for, and `passed` every one all three
// clauses let through, which is what each leaf is then asked of.
std::atomic<uint64_t> considered{0};
std::atomic<uint64_t> passed{0};
std::atomic<uint64_t> skipped_check{0};
std::atomic<uint64_t> skipped_mate{0};
std::atomic<uint64_t> skipped_score{0};

// A search that completed no iteration has no score and no line: the move it
// hands back is an aborted iteration's and is played, but nothing it said is
// written. Depth 1 completes at any budget a run uses; counted so a budget
// small enough to change that says so. S082.
std::atomic<uint64_t> incomplete{0};

// The coverage report the 2026-09-27 tuning comparison kept for this step
// (DEC-237): rows by the phase of the position written and the band the
// sampled position's score falls in. A print and not a resampling. The bands
// are reporting edges, not fitted values.
constexpr int PHASE_BANDS = 5;  // 0-4, 5-9, 10-14, 15-19, 20-24
constexpr int SCORE_BANDS = 5;  // |score| below 50, 150, 300, 600, the rest
constexpr int SCORE_BAND_EDGES[SCORE_BANDS - 1] = {50, 150, 300, 600};

int phase_band(int phase)
{ return std::min(std::max(phase, 0) / 5, PHASE_BANDS - 1); }

int score_band(int score)
{
  const int magnitude = std::abs(score);

  for (int band = 0; band < SCORE_BANDS - 1; ++band) {
    if (magnitude < SCORE_BAND_EDGES[band]) { return band; }
  }

  return SCORE_BANDS - 1;
}

// Per leaf: what was written, how far from the root, and why the rest was not.
struct leaf_stats_t
{
  std::atomic<uint64_t> recorded{0};
  std::atomic<uint64_t> differs{0};   // recorded with a distance above zero
  std::atomic<uint64_t> distance{0};  // summed over recorded rows
  std::atomic<uint64_t> pv_plies{0};  // the pv leaf's, summed likewise
  std::atomic<uint64_t> refused[LEAF_OUTCOME_COUNT];
  std::atomic<uint64_t> coverage[PHASE_BANDS][SCORE_BANDS];
};

leaf_stats_t leaf_stats[KIND_COUNT];

// Every node the walks searched, against the games' own, for the cost line.
std::atomic<uint64_t> walk_nodes{0};
std::atomic<uint64_t> game_nodes{0};


// Iterative deepening against a node budget, which is the same shape as the
// UCI layer's loop with everything that reports to a GUI taken out. The budget
// is spent across iterations rather than per iteration, so a game plays at a
// fixed cost per move whatever depth that happens to buy.
search_t run_search(game_t* game,
                    transposition_table_t* tt,
                    pawn_correction_t* correction,
                    nonpawn_correction_t* nonpawn_correction,
                    uint64_t nodes,
                    int max_depth)
{
  // Heap-owned, not a local: search_state_t is 1.2 MiB and play_games() calls
  // this from a worker std::thread, whose default stack is 512 KiB on macOS.
  // Value-initialised exactly as `= {}` was. S222.
  auto state_owner = std::make_unique<search_state_t>();
  search_state_t& state = *state_owner;
  state.stop = &never_stop;
  state.tt = tt;

  // The pawn correction table the UCI layer's searches carry, S099, so the
  // games written here are played by the engine a match plays and not by the
  // one before S099. Owned by the worker and cleared per game, as the UCI
  // layer clears its own on `ucinewgame`.
  state.pawn_correction = correction;

  // And S110's two non-pawn tables beside it, owned and cleared the same way.
  state.nonpawn_correction = nonpawn_correction;

  tt_new_search(tt);

  search_t best = {};
  uint64_t used = 0;

  for (int depth = 1; depth <= max_depth; ++depth) {
    state.explored_nodes = 0;
    state.node_limit = nodes - used;

    const search_t result = search(depth, game, &state);
    used += result.explored_nodes;

    // An aborted iteration still knows something the completed one did not: it
    // searches the previous best move first and only replaces it on a move that
    // beat every move before it. Its score is meaningless, which is why only a
    // completed iteration is allowed to set one.
    if (result.pv.length > 0) { best.best_move = result.best_move; }

    if (!state.aborted) {
      best.score = result.score;
      best.mate_found = result.mate_found;
      best.pv = result.pv;
    }

    if (state.aborted || used >= nodes) { break; }
  }

  best.explored_nodes = used;
  return best;
}


bool has_legal_move(game_t* game)
{
  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (make_move(game, moves[i])) {
      unmake_move(game);
      return true;
    }
  }

  return false;
}


// A uniform choice among the legal moves. Returns 0 when there are none, which
// is how a randomised opening that walked into mate is detected.
move_t random_move(game_t* game, std::mt19937_64& rng)
{
  move_t moves[MAX_MOVES];
  move_t legal[MAX_MOVES];

  const size_t count = generate_moves(game_tables(), &game->board, moves);
  size_t legal_count = 0;

  for (size_t i = 0; i < count; ++i) {
    if (make_move(game, moves[i])) {
      unmake_move(game);
      legal[legal_count++] = moves[i];
    }
  }

  if (legal_count == 0) { return 0; }

  std::uniform_int_distribution<size_t> pick(0, legal_count - 1);
  return legal[pick(rng)];
}


// 1.0, 0.5 or 0.0 from White's point of view, or -1 when the game is not over.
double terminal_result(game_t* game)
{
  if (game->board.halfmove_clock >= 100) { return 0.5; }
  if (is_insufficient_material(&game->board)) { return 0.5; }
  if (is_position_repeated(&game->history, &game->board)) { return 0.5; }

  if (!has_legal_move(game)) {
    // Stalemate is a draw; mate is a loss for the side that has to move.
    if (!is_check(game)) { return 0.5; }
    return (game->board.active_color == WHITE) ? 0.0 : 1.0;
  }

  return -1.0;
}


// The sampled position's half-move: the FEN's own reading of where the game
// stands, the value tools/tuner_split.hpp computes from a root row's FEN.
int sampled_ply(const board_t* board)
{
  return 2 * (static_cast<int>(board->fullmove_counter) - 1) +
         ((board->active_color == BLACK) ? 1 : 0);
}


// One sampled position, written as the leaf `kind` names. The game is left
// where it was: the walks unmake everything they make.
void sample_leaf(leaf_kind_t kind,
                 leaf_walker_t* walker,
                 game_t* game,
                 const search_t& found,
                 int white_score,
                 std::vector<sample_t>* out)
{
  leaf_stats_t& stats = leaf_stats[kind];

  sample_t sample = {};
  sample.score = white_score;
  sample.ply = sampled_ply(&game->board);

  if (kind == KIND_ROOT) {
    sample.fen = generate_FEN(&game->board);
    sample.phase = game_phase(&game->board);
    sample.distance = 0;
  } else {
    const leaf_walk_t walk = (kind == KIND_QS)
                                 ? leaf_walk_quiescence(walker, game)
                                 : leaf_walk_pv(walker, game, &found.pv);

    if (walk.outcome != LEAF_QUIET) {
      stats.refused[walk.outcome]++;
      return;
    }

    sample.fen = walk.fen;
    sample.phase = walk.phase;
    sample.distance = walk.distance;
    stats.pv_plies += static_cast<uint64_t>(walk.pv_plies);
  }

  stats.recorded++;
  stats.distance += static_cast<uint64_t>(sample.distance);
  if (sample.distance > 0) { stats.differs++; }
  stats.coverage[phase_band(sample.phase)][score_band(sample.score)]++;

  out->push_back(sample);
}


void play_games(const options_t& opts,
                unsigned index,
                const std::array<FILE*, KIND_COUNT>& outs)
{
  // Per worker, and not optional: the zobrist keys live in game_t, so a game
  // that skips this hashes every position to zero. Nothing complains — the
  // transposition table collapses onto one entry and repetition detection
  // reports a draw two plies into every game.
  game_t game;
  initialize_game_const_data(&game);

  transposition_table_t tt = {};
  tt_resize(&tt, static_cast<size_t>(opts.tt_mb));

  // On the heap for search_state_t's reason in run_search() -- this is a
  // worker thread -- though at the census's size it is 32 KiB. S099.
  auto correction = std::make_unique<pawn_correction_t>();

  // S110's two non-pawn tables, the same way: 256 KiB for the pair.
  auto nonpawn_correction = std::make_unique<nonpawn_correction_t>();

  std::mt19937_64 rng(opts.seed + index);

  const uint64_t share =
      opts.games / opts.threads + ((index < opts.games % opts.threads) ? 1 : 0);

  std::vector<sample_t> samples[KIND_COUNT];

  // The walks' own table and search state, apart from the game's: a leaf is a
  // function of the position and not of what this worker searched before it.
  // S082.
  leaf_walker_t walker;

  // A discarded opening costs an attempt and not a game, so the two are counted
  // separately. The attempt cap is only there so a broken filter cannot spin
  // forever; a run that hits it has a configuration problem, not a data one.
  const uint64_t attempt_cap = share * 100 + 1000;
  uint64_t attempts = 0;

  for (uint64_t kept = 0;
       kept < share && attempts < attempt_cap && !write_failed.load();) {
    attempts++;
    for (std::vector<sample_t>& kind_samples : samples) {
      kind_samples.clear();
    }
    tt_reset(&tt);
    correction_reset(correction.get());
    correction_reset(nonpawn_correction.get());

    if (!load_FEN(DEFAULT_POSITION, &game)) { break; }

    // Randomised opening. A game that mates itself on the way out of the book
    // is thrown away and replayed rather than recorded, so a discarded opening
    // costs an iteration and not a data point.
    bool usable = true;

    for (int i = 0; i < opts.random_plies && usable; ++i) {
      const move_t move = random_move(&game, rng);
      if (move == 0 || !make_move(&game, move)) { usable = false; }
    }

    if (!usable || terminal_result(&game) >= 0.0) { continue; }

    // An opening already decided teaches the evaluation about positions no
    // reasonable game reaches, and its label is fixed before a single move of
    // real play.
    const search_t opening =
        run_search(&game, &tt, correction.get(), nonpawn_correction.get(),
                   opts.nodes, MAX_DEPTH);

    if (opening.mate_found || std::abs(opening.score) > opts.opening_limit) {
      continue;
    }

    double result = -1.0;
    int decisive_run = 0;
    int decisive_sign = 0;

    for (int ply = 0; ply < opts.max_plies; ++ply) {
      result = terminal_result(&game);
      if (result >= 0.0) { break; }

      const search_t found =
          run_search(&game, &tt, correction.get(), nonpawn_correction.get(),
                     opts.nodes, MAX_DEPTH);

      if (found.best_move == 0) { break; }

      const bool white = (game.board.active_color == WHITE);
      const int white_score = white ? found.score : -found.score;
      game_nodes += found.explored_nodes;

      // Only a completed iteration leaves a line, so an empty one is a search
      // that completed none. S082.
      const bool completed = found.pv.length > 0;

      // Quiet, and inside the band where the score means a position rather
      // than a mate count. Three independent clauses, kept apart so each one
      // can be counted and so the score cap can move without touching the
      // rest.
      const bool not_in_check = !is_check(&game);
      const bool no_mate = !found.mate_found;
      const bool inside_band = std::abs(found.score) < opts.quiet_limit;

      if (!completed) {
        incomplete++;
      } else {
        considered++;
        if (!not_in_check && no_mate && inside_band) { skipped_check++; }
        if (not_in_check && !no_mate && inside_band) { skipped_mate++; }
        if (not_in_check && no_mate && !inside_band) { skipped_score++; }
      }

      if (completed && not_in_check && no_mate && inside_band) {
        passed++;

        const uint64_t nodes_before = walker.nodes;

        for (const leaf_kind_t kind : opts.leaves) {
          sample_leaf(kind, &walker, &game, found, white_score, &samples[kind]);
        }

        walk_nodes += walker.nodes - nodes_before;
      }

      // Adjudication. Playing out a position won by 20 pawns produces labels
      // that were settled long before and positions no evaluation needs help
      // with.
      const int sign = (white_score > 0) ? 1 : -1;

      if (found.mate_found || std::abs(white_score) >= opts.resign_score) {
        decisive_run = (sign == decisive_sign) ? decisive_run + 1 : 1;
        decisive_sign = sign;
      } else {
        decisive_run = 0;
        decisive_sign = 0;
      }

      if (decisive_run >= opts.resign_plies) {
        result = (decisive_sign > 0) ? 1.0 : 0.0;
        break;
      }

      if (!make_move(&game, found.best_move)) { break; }
    }

    // A game that ran out of plies without a verdict is scored a draw, which is
    // what a 400-ply game with neither side making progress amounts to.
    if (result < 0.0) { result = 0.5; }

    {
      const std::lock_guard<std::mutex> guard(output_lock);

      // Unique inside the run whatever the thread count, and the same game
      // under the same number in every leaf's file: worker `index` plays
      // games index, index + threads, index + 2 * threads and so on.
      const uint64_t game_id = kept * opts.threads + index;

      for (const leaf_kind_t kind : opts.leaves) {
        for (const sample_t& sample : samples[kind]) {
          if (fprintf(outs[kind], "%s\t%.1f\t%d\t%d\t%" PRIu64 "\t%d\t%d\n",
                      sample.fen.c_str(), result, sample.score, sample.phase,
                      game_id, sample.ply, sample.distance) < 0) {
            write_failed = true;
          }
        }

        if (fflush(outs[kind]) != 0) { write_failed = true; }
      }
    }

    kept++;
    games_done++;
  }

  tt_free(&tt);
}


// `root,qs,pv` or any non-empty subset, each kind once.
bool parse_leaves(const std::string& text, std::vector<leaf_kind_t>* leaves)
{
  leaves->clear();
  size_t start = 0;

  for (;;) {
    const size_t comma = text.find(',', start);
    const std::string name =
        text.substr(start, (comma == std::string::npos) ? std::string::npos
                                                        : comma - start);

    bool known = false;

    for (int kind = 0; kind < KIND_COUNT; ++kind) {
      if (name != KIND_NAMES[kind]) { continue; }

      const leaf_kind_t asked = static_cast<leaf_kind_t>(kind);

      if (std::find(leaves->begin(), leaves->end(), asked) != leaves->end()) {
        return false;
      }

      leaves->push_back(asked);
      known = true;
    }

    if (!known) { return false; }
    if (comma == std::string::npos) { break; }

    start = comma + 1;
  }

  return !leaves->empty();
}


// The file a kind is written to: `--out` itself for a run of one kind, and
// `--out` with the kind appended for a pool, so no two kinds share a file.
std::string leaf_path(const options_t& opts, leaf_kind_t kind)
{
  if (opts.leaves.size() == 1) { return opts.out; }
  return opts.out + "." + KIND_NAMES[kind];
}


double share_of(uint64_t part, uint64_t whole)
{
  return (whole == 0)
             ? 0.0
             : 100.0 * static_cast<double>(part) / static_cast<double>(whole);
}


void usage()
{
  fprintf(
      stderr,
      "datagen --out FILE [options]\n"
      "  --games N          games to play (default 1000)\n"
      "  --nodes N          node budget per move (default 5000)\n"
      "  --random-plies N   uniform random opening moves (default 8)\n"
      "  --max-plies N      give up on a game after this many (default 400)\n"
      "  --opening-limit N  discard an opening scored beyond this (400)\n"
      "  --resign-score N   adjudicate at this margin (default 2000)\n"
      "  --resign-plies N   ... held for this many plies (default 6)\n"
      "  --quiet-limit N    do not record beyond this score (default 1000)\n"
      "  --leaf LIST        which position a row names, comma separated:\n"
      "                     root, qs (default) or pv; more than one writes\n"
      "                     FILE.root, FILE.qs, FILE.pv from the same games\n"
      "  --threads N        worker threads (default: every hardware thread)\n"
      "  --seed N           rng seed (default 1)\n"
      "  --hash N           transposition table MB per thread (default 16)\n");
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;
  std::string leaf_list = "qs";

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    const bool has_value = (i + 1 < argc);

    if (arg == "--help") {
      usage();
      return 0;
    }

    if (!has_value) {
      usage();
      return 1;
    }

    const std::string value = argv[++i];

    if (arg == "--out") {
      opts.out = value;
    } else if (arg == "--games") {
      opts.games = strtoull(value.c_str(), nullptr, 10);
    } else if (arg == "--nodes") {
      opts.nodes = strtoull(value.c_str(), nullptr, 10);
    } else if (arg == "--random-plies") {
      opts.random_plies = atoi(value.c_str());
    } else if (arg == "--max-plies") {
      opts.max_plies = atoi(value.c_str());
    } else if (arg == "--opening-limit") {
      opts.opening_limit = atoi(value.c_str());
    } else if (arg == "--resign-score") {
      opts.resign_score = atoi(value.c_str());
    } else if (arg == "--resign-plies") {
      opts.resign_plies = atoi(value.c_str());
    } else if (arg == "--quiet-limit") {
      opts.quiet_limit = atoi(value.c_str());
    } else if (arg == "--leaf") {
      leaf_list = value;
    } else if (arg == "--threads") {
      opts.threads = static_cast<unsigned>(atoi(value.c_str()));
    } else if (arg == "--seed") {
      opts.seed = strtoull(value.c_str(), nullptr, 10);
    } else if (arg == "--hash") {
      opts.tt_mb = atoi(value.c_str());
    } else {
      // --allow-tactical lands here since S082: its clause is retired, and a
      // run that asks for it is asking for a corpus this tool no longer makes.
      usage();
      return 1;
    }
  }

  if (!parse_leaves(leaf_list, &opts.leaves)) {
    fprintf(stderr,
            "unusable --leaf '%s': a comma-separated list of root, qs and pv, "
            "each once\n",
            leaf_list.c_str());
    return 1;
  }

  if (opts.out.empty() || opts.threads == 0) {
    usage();
    return 1;
  }

  std::array<FILE*, KIND_COUNT> outs = {};

  for (const leaf_kind_t kind : opts.leaves) {
    const std::string path = leaf_path(opts, kind);
    outs[kind] = fopen(path.c_str(), "w");

    if (outs[kind] == nullptr) {
      fprintf(stderr, "cannot write %s\n", path.c_str());

      for (FILE* opened : outs) {
        if (opened != nullptr) { fclose(opened); }
      }

      return 1;
    }
  }

  fprintf(stderr,
          "datagen: %" PRIu64 " games, %" PRIu64
          " nodes per move, %u threads, "
          "seed %" PRIu64 ", quiet-limit %d, leaf %s\n",
          opts.games, opts.nodes, opts.threads, opts.seed, opts.quiet_limit,
          leaf_list.c_str());

  // Built once, on this thread, before anything can race for them. The tables
  // are process-wide since S007 and every worker reads the same copy.
  {
    game_t warmup;
    initialize_game_const_data(&warmup);
  }

  std::vector<std::thread> workers;

  for (unsigned i = 0; i < opts.threads; ++i) {
    workers.emplace_back(play_games, std::cref(opts), i, std::cref(outs));
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  for (FILE* out : outs) {
    if (out != nullptr && fclose(out) != 0) { write_failed = true; }
  }

  // What the filter cost, per clause. Each figure is what loosening that one
  // clause alone would have admitted, which is the only form of the number that
  // decides whether a clause is worth its exclusion.
  fprintf(stderr,
          "filter: %" PRIu64 " considered, %" PRIu64
          " passed; skipped in-check %" PRIu64 ", mate %" PRIu64
          ", score past %d %" PRIu64 "; no completed iteration %" PRIu64 "\n",
          considered.load(), passed.load(), skipped_check.load(),
          skipped_mate.load(), opts.quiet_limit, skipped_score.load(),
          incomplete.load());

  // Per leaf: what was written, how often it is not the root -- the share the
  // step's accepts asks for -- and why the rest of what passed was not.
  for (const leaf_kind_t kind : opts.leaves) {
    const leaf_stats_t& stats = leaf_stats[kind];
    const uint64_t recorded = stats.recorded.load();
    const double mean_distance =
        (recorded == 0) ? 0.0
                        : static_cast<double>(stats.distance.load()) /
                              static_cast<double>(recorded);

    fprintf(stderr,
            "leaf %s: %" PRIu64 " rows from %" PRIu64
            " games written to %s, %.2f a game; the leaf differs from the root "
            "on %" PRIu64 " (%.2f %%), %.3f plies on average",
            KIND_NAMES[kind], recorded, games_done.load(),
            leaf_path(opts, kind).c_str(),
            (games_done.load() == 0)
                ? 0.0
                : static_cast<double>(recorded) /
                      static_cast<double>(games_done.load()),
            stats.differs.load(), share_of(stats.differs.load(), recorded),
            mean_distance);

    if (kind == KIND_PV) {
      fprintf(stderr, ", %.3f of them the line's",
              (recorded == 0) ? 0.0
                              : static_cast<double>(stats.pv_plies.load()) /
                                    static_cast<double>(recorded));
    }

    fprintf(stderr, "\n");

    if (kind != KIND_ROOT) {
      fprintf(stderr,
              "leaf %s: not written of %" PRIu64 " passed:", KIND_NAMES[kind],
              passed.load());

      for (int outcome = LEAF_TERMINAL; outcome < LEAF_OUTCOME_COUNT;
           ++outcome) {
        const uint64_t count = stats.refused[outcome].load();
        fprintf(stderr, " %s %" PRIu64 " (%.4f %%)",
                leaf_outcome_name(static_cast<leaf_outcome_t>(outcome)), count,
                share_of(count, passed.load()));
      }

      fprintf(stderr, "\n");
    }

    fprintf(stderr,
            "coverage %s, rows by the written position's phase and the "
            "sampled position's |score|\n  %-11s %10s %10s %10s %10s %10s\n",
            KIND_NAMES[kind], "phase", "<50", "<150", "<300", "<600", "rest");

    for (int phase = 0; phase < PHASE_BANDS; ++phase) {
      fprintf(stderr, "  %5d to %2d", phase * 5,
              (phase == PHASE_BANDS - 1) ? GAME_PHASE_MAX : phase * 5 + 4);

      for (int band = 0; band < SCORE_BANDS; ++band) {
        fprintf(stderr, " %10" PRIu64, stats.coverage[phase][band].load());
      }

      fprintf(stderr, "\n");
    }
  }

  // What the walks cost against the games they were taken from, in nodes,
  // which is the machine's time without the clock's noise.
  fprintf(stderr,
          "walk: %" PRIu64 " nodes, %.3f %% of the %" PRIu64
          " the games' searches spent\n",
          walk_nodes.load(), share_of(walk_nodes.load(), game_nodes.load()),
          game_nodes.load());

  // The two ways the files can fall short of what was booked, each a failed
  // run: the lines above are kept, since they say how far it got.
  int status = 0;

  if (write_failed.load()) {
    fprintf(stderr,
            "datagen: FAILED, a write or flush to an output file failed; the "
            "files hold fewer rows than the counts above\n");
    status = 1;
  }

  if (games_done.load() < opts.games) {
    fprintf(stderr,
            "datagen: FAILED, %" PRIu64 " of %" PRIu64
            " games played -- a worker stopped short, at its attempt cap or "
            "on a failed write\n",
            games_done.load(), opts.games);
    status = 1;
  }

  return status;
}
