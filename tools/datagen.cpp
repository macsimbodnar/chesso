// Self-play position generator for evaluation tuning. S028.
//
// Plays chesso against itself at a fixed node budget from a randomised opening
// and writes, one per line:
//
//   fen  result  score  phase
//
// `result` is the game's outcome from White's point of view, 1 win, 0.5 draw,
// 0 loss. `score` is the engine's own score for the position, White relative,
// and `phase` its game_phase(). The tuner needs the first two; the other two
// are written so a dataset can be re-filtered without replaying anything.
//
// Four independent filter clauses decide whether a position is recorded: the
// side to move is not in check, the search found no mate, the score is inside
// --quiet-limit, and -- unless --allow-tactical is set -- the move the search
// chose is neither a capture nor a promotion.
//
// That fourth clause was justified here by the claim that evaluate() is only
// ever asked about a position quiescence has already resolved. It is not:
// src/search.cpp:125 calls evaluate_lazy() at the top of every quiescence node,
// before a single capture is generated, and that stand-pat score is the bound
// the node returns on. The static evaluation is asked at exactly the positions
// this clause excludes, which is why it is a flag and no longer a rule.
// DEC-055.
//
// Every run prints what each clause cost, so the next corpus is sized from a
// measurement rather than from this comment.
//
// Nothing here reads another engine. The positions are chesso's own play and
// the labels are game outcomes, which is what DEC-016 requires of training
// data.
#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "evaluation.hpp"
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
  // 0 keeps the S028 behaviour: a position whose best move is a capture or a
  // promotion is not recorded. 1 records it anyway. DEC-055.
  int allow_tactical = 0;
  unsigned threads = default_threads();
  uint64_t seed = 1;
  int tt_mb = 16;
  std::string out = "";
};

struct sample_t
{
  std::string fen;
  int score;  // White relative
  int phase;
};

// search() polls this and never sets it. Datagen has no clock, so it is only
// ever false; the node budget is what stops a search.
std::atomic_bool never_stop{false};

std::mutex output_lock;
std::atomic<uint64_t> games_done{0};
std::atomic<uint64_t> positions_done{0};

// What the filter threw away, counted per clause and printed at the end. The
// clauses are independent tests, not a chain: each counter below is the number
// of positions that would have been recorded but for that one clause, so it
// prices exactly what loosening that clause alone would admit. A position
// failing two clauses is in neither count. `considered` is every position a
// completed search returned a move for.
std::atomic<uint64_t> considered{0};
std::atomic<uint64_t> skipped_check{0};
std::atomic<uint64_t> skipped_tactical{0};
std::atomic<uint64_t> skipped_mate{0};
std::atomic<uint64_t> skipped_score{0};


// Iterative deepening against a node budget, which is the same shape as the
// UCI layer's loop with everything that reports to a GUI taken out. The budget
// is spent across iterations rather than per iteration, so a game plays at a
// fixed cost per move whatever depth that happens to buy.
search_t run_search(game_t* game,
                    transposition_table_t* tt,
                    uint64_t nodes,
                    int max_depth)
{
  search_state_t state = {};
  state.stop = &never_stop;
  state.tt = tt;

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


void play_games(const options_t& opts, unsigned index, FILE* out)
{
  // Per worker, and not optional: the zobrist keys live in game_t, so a game
  // that skips this hashes every position to zero. Nothing complains — the
  // transposition table collapses onto one entry and repetition detection
  // reports a draw two plies into every game.
  game_t game;
  initialize_game_const_data(&game);

  transposition_table_t tt = {};
  tt_resize(&tt, static_cast<size_t>(opts.tt_mb));

  std::mt19937_64 rng(opts.seed + index);

  const uint64_t share =
      opts.games / opts.threads + ((index < opts.games % opts.threads) ? 1 : 0);

  std::vector<sample_t> samples;

  // A discarded opening costs an attempt and not a game, so the two are counted
  // separately. The attempt cap is only there so a broken filter cannot spin
  // forever; a run that hits it has a configuration problem, not a data one.
  const uint64_t attempt_cap = share * 100 + 1000;
  uint64_t attempts = 0;

  for (uint64_t kept = 0; kept < share && attempts < attempt_cap;) {
    attempts++;
    samples.clear();
    tt_reset(&tt);

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
    const search_t opening = run_search(&game, &tt, opts.nodes, MAX_DEPTH);

    if (opening.mate_found || std::abs(opening.score) > opts.opening_limit) {
      continue;
    }

    double result = -1.0;
    int decisive_run = 0;
    int decisive_sign = 0;

    for (int ply = 0; ply < opts.max_plies; ++ply) {
      result = terminal_result(&game);
      if (result >= 0.0) { break; }

      const search_t found = run_search(&game, &tt, opts.nodes, MAX_DEPTH);

      if (found.best_move == 0) { break; }

      const bool white = (game.board.active_color == WHITE);
      const int white_score = white ? found.score : -found.score;

      // Quiet, and inside the band where the score means a position rather
      // than a mate count. Four independent clauses, kept apart so each one can
      // be counted and so the score cap can move without touching the rest.
      const bool not_in_check = !is_check(&game);
      const bool move_is_quiet =
          (opts.allow_tactical != 0) ||
          (!MOVE_CAPTURE(found.best_move) && !MOVE_PROMOTED(found.best_move));
      const bool no_mate = !found.mate_found;
      const bool inside_band = std::abs(found.score) < opts.quiet_limit;

      const bool quiet =
          not_in_check && move_is_quiet && no_mate && inside_band;

      considered++;
      if (!not_in_check && move_is_quiet && no_mate && inside_band) {
        skipped_check++;
      }
      if (not_in_check && !move_is_quiet && no_mate && inside_band) {
        skipped_tactical++;
      }
      if (not_in_check && move_is_quiet && !no_mate && inside_band) {
        skipped_mate++;
      }
      if (not_in_check && move_is_quiet && no_mate && !inside_band) {
        skipped_score++;
      }

      if (quiet) {
        samples.push_back(
            {generate_FEN(&game.board), white_score, game_phase(&game.board)});
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

      for (const sample_t& sample : samples) {
        fprintf(out, "%s\t%.1f\t%d\t%d\n", sample.fen.c_str(), result,
                sample.score, sample.phase);
      }

      fflush(out);
    }

    kept++;
    games_done++;
    positions_done += samples.size();
  }

  tt_free(&tt);
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
      "  --allow-tactical N 1 records a position whose best move is a capture\n"
      "                     or a promotion, 0 excludes it (default 0)\n"
      "  --threads N        worker threads (default: every hardware thread)\n"
      "  --seed N           rng seed (default 1)\n"
      "  --hash N           transposition table MB per thread (default 16)\n");
}

}  // namespace


int main(int argc, char** argv)
{
  options_t opts;

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
    } else if (arg == "--allow-tactical") {
      opts.allow_tactical = atoi(value.c_str());
    } else if (arg == "--threads") {
      opts.threads = static_cast<unsigned>(atoi(value.c_str()));
    } else if (arg == "--seed") {
      opts.seed = strtoull(value.c_str(), nullptr, 10);
    } else if (arg == "--hash") {
      opts.tt_mb = atoi(value.c_str());
    } else {
      usage();
      return 1;
    }
  }

  if (opts.out.empty() || opts.threads == 0) {
    usage();
    return 1;
  }

  FILE* out = fopen(opts.out.c_str(), "w");

  if (out == nullptr) {
    fprintf(stderr, "cannot write %s\n", opts.out.c_str());
    return 1;
  }

  fprintf(stderr,
          "datagen: %" PRIu64 " games, %" PRIu64
          " nodes per move, %u threads, "
          "seed %" PRIu64 ", quiet-limit %d, allow-tactical %d\n",
          opts.games, opts.nodes, opts.threads, opts.seed, opts.quiet_limit,
          opts.allow_tactical);

  // Built once, on this thread, before anything can race for them. The tables
  // are process-wide since S007 and every worker reads the same copy.
  {
    game_t warmup;
    initialize_game_const_data(&warmup);
  }

  std::vector<std::thread> workers;

  for (unsigned i = 0; i < opts.threads; ++i) {
    workers.emplace_back(play_games, std::cref(opts), i, out);
  }

  for (std::thread& worker : workers) {
    worker.join();
  }

  fclose(out);

  fprintf(stderr, "%" PRIu64 " games, %" PRIu64 " positions written to %s\n",
          games_done.load(), positions_done.load(), opts.out.c_str());

  // What the filter cost, per clause. Each figure is what loosening that one
  // clause alone would have admitted, which is the only form of the number that
  // decides whether a clause is worth its exclusion.
  fprintf(stderr,
          "filter: %" PRIu64 " considered, %" PRIu64
          " recorded; skipped "
          "in-check %" PRIu64 ", tactical best move %" PRIu64 ", mate %" PRIu64
          ", score past %d %" PRIu64 "\n",
          considered.load(), positions_done.load(), skipped_check.load(),
          skipped_tactical.load(), skipped_mate.load(), opts.quiet_limit,
          skipped_score.load());

  return 0;
}
