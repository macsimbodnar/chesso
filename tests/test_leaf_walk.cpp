#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "leaf_walk.hpp"
#include "search.hpp"
#include "search_params.hpp"
#include "test_helpers.hpp"
#include "transposition_table.hpp"

// S082: the corpus row names the leaf quiescence resolves a position to, and
// the leaf is quiet by construction. What this file holds:
//
//   1. every quiet leaf the walk records is a position quiescence stands pat
//      at, read back from its FEN alone in a game and a table the walk never
//      touched -- no capture or promotion quiescence would still make there;
//      every walk that left its root left one quiescence does not stand pat
//      at; and the walk leaves the game where it found it
//   2. a walk stopped by its ply bound is refused as truncated, never recorded
//      as a leaf, and where it stopped quiescence still makes a move -- so the
//      two endings are told apart, which a test of the quiet ending alone
//      cannot do
//   3. the default bound is MAX_QSEARCH_DEPTH, quiescence's own horizon
//   4. a line that ends in mate is terminal, not quiet
//   5. a principal variation played out ends at a quiet leaf, resolved past
//      its last move where that move leaves something to resolve -- built so
//      it must, from the quiescence walk's own first move -- and a line the
//      generator does not produce here is refused
//   6. a repetition refuses a leaf exactly where the search would score it a
//      draw from the sampled position: inside the line, and not for a single
//      occurrence before the root, which the game-level rule would refuse
//   7. a call that runs out of the walker's node cap is refused as aborted,
//      stopped on exactly the node that reached the cap, and the game is left
//      where it was -- the policy quiescence's own limit check drives (S279)
//
// tools/mutants/S082_leaf_walk.py lists the mutants these kill.

namespace
{

game_t walk_game;
game_t fresh_game;
transposition_table_t fresh_table;
transposition_table_t search_table;
std::atomic_bool never_stop{false};


// Every position the cases scan: the JSON suites, which hold captures,
// promotions and mates in quantity, and three busy positions the search tests
// already use for quiescence ("quiescence stays bounded").
const std::vector<std::string>& walk_positions()
{
  static const std::vector<std::string> positions = [] {
    std::vector<std::string> out = all_test_fens();

    out.push_back("4R1k1/5ppp/8/8/q7/8/8/4R1K1 b - - 0 1");
    out.push_back(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    out.push_back("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

    return out;
  }();

  return positions;
}


// What quiescence() makes of a position asked from its FEN alone: a game of
// its own, a table of its own cleared first, a search state of its own, the
// widest window. The walk is not consulted, so this is the case's own reading
// of the property and not the walk's reading of itself.
struct reading_t
{
  bool in_check = false;
  int value = 0;
  int static_eval = 0;
};

reading_t read_fresh(const std::string& fen)
{
  initialize_game_const_data(&fresh_game);
  REQUIRE_MESSAGE(load_FEN(fen, &fresh_game), ("FEN: " + fen));

  if (fresh_table.clusters == nullptr) { tt_resize(&fresh_table, 1); }
  tt_reset(&fresh_table);

  auto state = std::make_unique<search_state_t>();
  state->tt = &fresh_table;
  state->stop = &never_stop;

  reading_t reading;
  reading.in_check = is_check(&fresh_game);
  reading.static_eval = evaluate(&fresh_game.board);
  reading.value = quiescence(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 0, 0,
                             &fresh_game, state.get());

  REQUIRE_FALSE_MESSAGE(state->aborted, ("FEN: " + fen));
  return reading;
}


// Quiescence stands pat here: out of check, and nothing it searches beats the
// static score. A capture it would still make raises the value above it.
bool stands_pat(const std::string& fen)
{
  const reading_t reading = read_fresh(fen);
  return !reading.in_check && reading.value == reading.static_eval;
}


// Whether the rules have decided the position the FEN names, by the engine's
// own tests: mate or stalemate, or material that cannot mate.
bool rules_decide_fen(const std::string& fen)
{
  initialize_game_const_data(&fresh_game);
  REQUIRE_MESSAGE(load_FEN(fen, &fresh_game), ("FEN: " + fen));

  move_t moves[MAX_MOVES];
  return legal_moves(&fresh_game, moves) == 0 ||
         is_insufficient_material(&fresh_game.board);
}


void load_walk_game(const std::string& fen)
{
  initialize_game_const_data(&walk_game);
  REQUIRE_MESSAGE(load_FEN(fen, &walk_game), ("FEN: " + fen));
}


// The legal move from one square to another where the game stands, by the
// engine's generator; REQUIREs that there is one.
move_t find_move(game_t* game, const std::string& from, const std::string& to)
{
  move_t moves[MAX_MOVES];
  const size_t count = legal_moves(game, moves);

  for (size_t i = 0; i < count; ++i) {
    if (MOVE_FROM(moves[i]) == str_to_index(from) &&
        MOVE_TO(moves[i]) == str_to_index(to)) {
      return moves[i];
    }
  }

  FAIL("no legal move " << from << to << " in " << generate_FEN(&game->board));
  return 0;
}


// A line of from-to pairs as a pv_t, each move found where the line stands;
// the game is left where it was.
pv_t line_of(game_t* game,
             const std::vector<std::pair<std::string, std::string>>& moves)
{
  pv_t line = {};

  for (const auto& [from, to] : moves) {
    const move_t move = find_move(game, from, to);
    REQUIRE(make_move(game, move));
    line.table[line.length++] = move;
  }

  for (size_t i = 0; i < line.length; ++i) {
    unmake_move(game);
  }

  return line;
}


// Placement, side to move, castling and en passant: the position a FEN names.
std::string position_of(const std::string& fen)
{
  const std::vector<std::string> parts = split_string(fen);
  REQUIRE(parts.size() >= 4);
  return parts[0] + " " + parts[1] + " " + parts[2] + " " + parts[3];
}

}  // namespace


TEST_SUITE("leaf walk")
{
  TEST_CASE("every quiet leaf the walk records is one quiescence stands pat at")
  {
    leaf_walker_t walker;

    size_t outcomes[LEAF_OUTCOME_COUNT] = {};
    size_t left_root = 0;
    size_t two_or_more = 0;

    for (const std::string& fen : walk_positions()) {
      load_walk_game(fen);

      const std::string before = generate_FEN(&walk_game.board);
      const uint64_t hash_before = walk_game.board.hash;
      const size_t history_before = walk_game.history.size;

      const leaf_walk_t walk = leaf_walk_quiescence(&walker, &walk_game);
      outcomes[walk.outcome]++;

      // Where it found the game, history included.
      REQUIRE_EQ(generate_FEN(&walk_game.board), before);
      REQUIRE_EQ(walk_game.board.hash, hash_before);
      REQUIRE_EQ(walk_game.history.size, history_before);

      if (walk.outcome != LEAF_QUIET) { continue; }

      // The property, read back from the recorded FEN alone: no capture or
      // promotion quiescence would still make.
      CHECK_MESSAGE(stands_pat(walk.fen),
                    ("leaf " + walk.fen + " of " + fen + " is not quiet"));

      if (walk.distance == 0) {
        CHECK_EQ(walk.fen, before);
        continue;
      }

      // The non-vacuity half: the walk moved only where the root itself was
      // not a position quiescence stands pat at.
      left_root++;
      if (walk.distance >= 2) { two_or_more++; }

      CHECK_MESSAGE(!stands_pat(fen),
                    ("the walk left " + fen + ", where quiescence stands pat"));
    }

    // A set with no tactics in it would pass the property over nothing.
    REQUIRE(left_root > 0);
    REQUIRE(two_or_more > 0);
    REQUIRE(outcomes[LEAF_QUIET] > left_root);

    // Neither refusal is reachable from a legal position on a cleared table;
    // a count above zero is a walk that stopped trusting its own answer.
    CHECK_EQ(outcomes[LEAF_ABORTED], 0);
    CHECK_EQ(outcomes[LEAF_UNCERTIFIED], 0);
  }


  TEST_CASE("a walk stopped by its ply bound is refused as truncated")
  {
    leaf_walker_t walker;
    leaf_walker_t bounded;

    size_t cases = 0;

    for (const std::string& fen : walk_positions()) {
      load_walk_game(fen);

      const leaf_walk_t walk = leaf_walk_quiescence(&walker, &walk_game);

      if (walk.outcome != LEAF_QUIET || walk.distance < 2) { continue; }

      // The same walk, one ply short of the leaf it reaches unbounded.
      bounded.max_plies = walk.distance - 1;
      const leaf_walk_t cut = leaf_walk_quiescence(&bounded, &walk_game);

      CHECK_MESSAGE(cut.outcome == LEAF_TRUNCATED,
                    ("bounded walk from " + fen + " ended " +
                     leaf_outcome_name(cut.outcome)));
      CHECK_EQ(cut.distance, walk.distance - 1);

      // What makes the refusal mean something: where the bound stopped it,
      // quiescence still makes a move, so recording that position would
      // record one that is not quiet.
      CHECK_MESSAGE(!stands_pat(cut.fen), ("the bound stopped at " + cut.fen +
                                           ", which quiescence stands pat at"));
      cases++;
    }

    REQUIRE(cases > 0);
  }


  TEST_CASE("the ply bound is quiescence's own horizon unless narrowed")
  {
    leaf_walker_t walker;
    CHECK_EQ(leaf_walk_ply_bound(&walker), MAX_QSEARCH_DEPTH);

    walker.max_plies = 3;
    CHECK_EQ(leaf_walk_ply_bound(&walker), 3);
  }


  // Black is in check with one legal reply, and the search tests record that
  // the line from here is forced to mate ("a side in check may not stand
  // pat"); the precondition below reads that from quiescence rather than from
  // the comment.
  TEST_CASE("a line that ends in mate is terminal, not quiet")
  {
    const std::string fen = "4R1k1/5ppp/8/8/q7/8/8/4R1K1 b - - 0 1";

    const reading_t root = read_fresh(fen);
    REQUIRE(root.in_check);
    REQUIRE(root.value < -10000);

    leaf_walker_t walker;
    load_walk_game(fen);

    const leaf_walk_t walk = leaf_walk_quiescence(&walker, &walk_game);

    CHECK_MESSAGE(walk.outcome == LEAF_TERMINAL,
                  ("ended " + std::string(leaf_outcome_name(walk.outcome)) +
                   " at " + walk.fen));
    CHECK(walk.distance > 0);
    CHECK(rules_decide_fen(walk.fen));

    // And a position the rules have already decided is refused where it
    // stands, without a move.
    load_walk_game(walk.fen);
    const leaf_walk_t again = leaf_walk_quiescence(&walker, &walk_game);
    CHECK_EQ(again.outcome, LEAF_TERMINAL);
    CHECK_EQ(again.distance, 0);
  }


  TEST_CASE("a principal variation is played out and then resolved")
  {
    leaf_walker_t walker;

    size_t resolved = 0;
    size_t past_the_line = 0;
    size_t outcomes[LEAF_OUTCOME_COUNT] = {};

    if (search_table.clusters == nullptr) { tt_resize(&search_table, 4); }

    // A slice is enough for the property, and a search per position is the
    // cost of this half: the suites' first 120 and the three busy positions
    // walk_positions() appends after them.
    const std::vector<std::string>& all = walk_positions();
    std::vector<std::string> positions(
        all.begin(), all.begin() + std::min<size_t>(120, all.size()));
    positions.insert(positions.end(), all.end() - 3, all.end());

    for (const std::string& fen : positions) {
      load_walk_game(fen);

      tt_reset(&search_table);
      tt_new_search(&search_table);

      auto state = std::make_unique<search_state_t>();
      state->tt = &search_table;
      state->stop = &never_stop;

      const search_t found = search(3, &walk_game, state.get());

      if (found.pv.length == 0) { continue; }

      const std::string before = generate_FEN(&walk_game.board);
      const leaf_walk_t walk = leaf_walk_pv(&walker, &walk_game, &found.pv);
      outcomes[walk.outcome]++;

      REQUIRE_EQ(generate_FEN(&walk_game.board), before);

      if (walk.outcome != LEAF_QUIET) { continue; }

      resolved++;
      CHECK_EQ(static_cast<size_t>(walk.pv_plies), found.pv.length);
      CHECK(walk.distance >= walk.pv_plies);
      CHECK_MESSAGE(stands_pat(walk.fen),
                    ("PV leaf " + walk.fen + " of " + fen + " is not quiet"));

      if (walk.distance > walk.pv_plies) { past_the_line++; }
    }

    REQUIRE(resolved > 0);
    CHECK_EQ(outcomes[LEAF_ABORTED], 0);
    CHECK_EQ(outcomes[LEAF_UNCERTIFIED], 0);

    // The half that cannot pass without resolving past the line. From every
    // position whose quiescence walk takes two plies or more, that walk's own
    // first move is handed over as a one-move line: the line's end is a
    // position quiescence still moves from, so the pv walk has to resolve it,
    // and it must land where the quiescence walk did. Whether such positions
    // exist is the engine's answer, the same one the first case requires.
    size_t constructed = 0;

    for (const std::string& fen : walk_positions()) {
      load_walk_game(fen);

      const leaf_walk_t qs = leaf_walk_quiescence(&walker, &walk_game);
      if (qs.outcome != LEAF_QUIET || qs.distance < 2) { continue; }

      int value = 0;
      move_t first = 0;
      bool aborted = false;
      REQUIRE(leaf_quiescence_choice(&walker, &walk_game, &value, &first,
                                     &aborted));
      REQUIRE(first != 0);

      pv_t line = {};
      line.length = 1;
      line.table[0] = first;

      const leaf_walk_t pv = leaf_walk_pv(&walker, &walk_game, &line);

      CHECK_MESSAGE(pv.outcome == LEAF_QUIET,
                    ("one-move line from " + fen + " ended " +
                     leaf_outcome_name(pv.outcome)));
      CHECK_EQ(pv.pv_plies, 1);
      CHECK_EQ(pv.distance, qs.distance);
      CHECK_EQ(pv.fen, qs.fen);

      constructed++;
      if (pv.outcome == LEAF_QUIET && pv.distance > pv.pv_plies) {
        past_the_line++;
      }
    }

    REQUIRE(constructed > 0);
    REQUIRE(past_the_line > 0);
  }


  TEST_CASE("a repetition refuses a leaf only where the search draws it")
  {
    // King and rook against king, the rook shuffling between d1 and d2 and
    // the king between e8 and f8. Every move is found by the engine's
    // generator (find_move() requires it) and none captures, so quiescence
    // stands pat wherever a line ends unless a rule has decided it.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";
    leaf_walker_t walker;

    load_walk_game(fen);
    REQUIRE_FALSE(is_insufficient_material(&walk_game.board));

    // One occurrence before the root. Two plies are played on the game, and
    // the line from there returns to where the game started.
    REQUIRE(make_move(&walk_game, find_move(&walk_game, "d1", "d2")));
    REQUIRE(make_move(&walk_game, find_move(&walk_game, "e8", "f8")));

    const pv_t back = line_of(&walk_game, {{"d2", "d1"}, {"f8", "e8"}});

    // The precondition: the game-level rule, a two-fold anywhere, does refuse
    // the position that line ends on.
    REQUIRE(make_move(&walk_game, back.table[0]));
    REQUIRE(make_move(&walk_game, back.table[1]));
    REQUIRE(is_position_repeated(&walk_game.history, &walk_game.board));
    unmake_move(&walk_game);
    unmake_move(&walk_game);

    const leaf_walk_t once = leaf_walk_pv(&walker, &walk_game, &back);
    CHECK_MESSAGE(once.outcome == LEAF_QUIET,
                  ("ended " + std::string(leaf_outcome_name(once.outcome))));
    CHECK_EQ(once.pv_plies, 2);
    CHECK_EQ(position_of(once.fen), position_of(fen));

    // The root's own position recurring once inside the line is the same
    // single occurrence, and the search does not draw it either.
    load_walk_game(fen);
    const pv_t round = line_of(
        &walk_game, {{"d1", "d2"}, {"e8", "f8"}, {"d2", "d1"}, {"f8", "e8"}});
    const leaf_walk_t returned = leaf_walk_pv(&walker, &walk_game, &round);
    CHECK_EQ(returned.outcome, LEAF_QUIET);
    CHECK_EQ(returned.pv_plies, 4);

    // A position repeated inside the line is a draw to the search, and the
    // line is refused there: the fifth ply stands where the first did.
    const pv_t again = line_of(
        &walk_game,
        {{"d1", "d2"}, {"e8", "f8"}, {"d2", "d1"}, {"f8", "e8"}, {"d1", "d2"}});
    const leaf_walk_t twice = leaf_walk_pv(&walker, &walk_game, &again);
    CHECK_EQ(twice.outcome, LEAF_TERMINAL);
    CHECK_EQ(twice.pv_plies, 5);
  }


  TEST_CASE("a line the generator does not produce here is refused")
  {
    leaf_walker_t walker;

    // A move from the start position, handed to a position it does not fit.
    load_walk_game(DEFAULT_POSITION);
    move_t start_moves[MAX_MOVES];
    REQUIRE(legal_moves(&walk_game, start_moves) > 0);
    const move_t foreign = start_moves[0];

    load_walk_game("4k3/8/8/8/8/8/8/3RK3 w - - 0 1");
    REQUIRE_FALSE(move_is_legal(&walk_game, foreign));

    const std::string before = generate_FEN(&walk_game.board);

    pv_t line = {};
    line.length = 1;
    line.table[0] = foreign;

    const leaf_walk_t walk = leaf_walk_pv(&walker, &walk_game, &line);

    CHECK_EQ(walk.outcome, LEAF_UNCERTIFIED);
    CHECK_EQ(walk.pv_plies, 0);
    CHECK_EQ(walk.distance, 0);
    CHECK_EQ(generate_FEN(&walk_game.board), before);
  }


  // The cap is the only limit a walker's search state carries: its stop flag
  // is its own and nothing sets it. Since S279 quiescence() tests the cap at
  // every node it counts, above its table probe, so a call stops on the node
  // that reaches it and every node above returns without counting another.
  // The aborted call's score is a placeholder, and the walk refuses the row
  // rather than read it or the table the call left.
  TEST_CASE("a call past the walker's node cap is refused as aborted")
  {
    leaf_walker_t walker;
    leaf_walker_t capped;

    size_t cases = 0;

    for (const std::string& fen : walk_positions()) {
      load_walk_game(fen);

      // The precondition, read from the engine: uncapped, the walk from here
      // is quiet, so the rules do not decide its root and the walk does call
      // quiescence there; and that first call completes having searched more
      // than one node, so a cap one node short of it has to bind.
      const leaf_walk_t full = leaf_walk_quiescence(&walker, &walk_game);
      if (full.outcome != LEAF_QUIET) { continue; }

      const uint64_t before_call = walker.nodes;
      int value = 0;
      move_t chosen = 0;
      bool aborted = false;
      const bool answered = leaf_quiescence_choice(&walker, &walk_game, &value,
                                                   &chosen, &aborted);
      const uint64_t searched = walker.nodes - before_call;

      REQUIRE_MESSAGE(answered, ("FEN: " + fen));
      REQUIRE_FALSE(aborted);
      if (searched < 2) { continue; }

      const std::string root = generate_FEN(&walk_game.board);
      const size_t history_before = walk_game.history.size;

      capped.node_cap = searched - 1;
      const uint64_t before_capped = capped.nodes;
      const leaf_walk_t cut = leaf_walk_quiescence(&capped, &walk_game);

      CHECK_MESSAGE(cut.outcome == LEAF_ABORTED,
                    ("capped walk from " + fen + " ended " +
                     leaf_outcome_name(cut.outcome)));
      CHECK_EQ(cut.distance, 0);
      CHECK_EQ(cut.fen, root);
      CHECK_EQ(capped.nodes - before_capped, capped.node_cap);

      REQUIRE_EQ(generate_FEN(&walk_game.board), root);
      REQUIRE_EQ(walk_game.history.size, history_before);

      // A slice is enough; each case costs a whole walk and two calls.
      if (++cases == 32) { break; }
    }

    REQUIRE(cases > 0);
  }
}
