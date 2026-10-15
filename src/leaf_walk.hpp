#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include "data_structures.hpp"

// Where a self-play position resolves to, for the tuning corpus. S082.
//
// The tuner fits evaluate() to game outcomes, and evaluate() is only
// well-defined on a position no capture changes: quiescence asks it at every
// node, but what a node finally hands its parent is the value of the position
// its best captures lead to. So the corpus row names that position -- the leaf
// -- rather than the one the game happened to stand on, and the game result
// stays the label. CPW, Texel's Tuning Method, defines the objective over the
// quiescence score; this is the same anchoring reached from the position's
// side.
//
// THE WALK. From the position it is handed, quiescence() is called once on a
// table cleared for that call, with the widest window, and the move it chose
// is read back from the exact entry the call's own root stored -- the last
// store of the call, so nothing below it can have overwritten it. No move
// means quiescence stood pat: no capture or promotion it searches beats the
// static score. The walk then plays the chosen move and asks again, one fresh
// call a ply, until quiescence stands pat. That position is the leaf.
//
// Nothing here changes what quiescence does, and nothing the engine plays
// calls this file: `chesso` does not link it. The walk asks quiescence() the
// question a test asks it, on its own table and its own search state, so the
// line it follows is quiescence's own -- its move set, its exchange gate, its
// ordering, its ties -- and no copy of any of that lives here.
//
// THE POLICIES, each a refusal the caller counts rather than a row it keeps:
//
//   terminal     the line reached a position the rules decide as the search
//                sees it from the sampled position -- checkmate, stalemate,
//                insufficient material, the fifty-move rule, or a repetition
//                classify_repetition() calls a draw with the sampled position
//                as its root: one inside the walk, or a third occurrence.
//                evaluate() is never the final word there, so the row would
//                fit a number to a draw or a mate. A single occurrence before
//                the root is not terminal: the search goes on searching that
//                position and evaluate() is asked about it, so refusing it
//                would drop a real leaf, refused for the game's history and
//                not its own. A policy outcome, never a sign of a broken walk.
//   truncated    the walk made its ply bound without reaching a position
//                quiescence stands pat at. The bound is MAX_QSEARCH_DEPTH,
//                the horizon one quiescence call has, so a walk is never
//                longer than the line a single call could have followed. A
//                leaf reached by exhausting it is not quiet, and recording it
//                puts back the noise the leaf removes -- the step's trap.
//   aborted      a quiescence call ran out of the walk's node cap, which
//                quiescence() tests at every node it counts (S279), so the
//                call stops on the node that reaches it. The call's answer is
//                a placeholder in place of a search: 0 from that node, a
//                stand pat from every node above it.
//   uncertified  the call's root entry is not exact, or names no move the
//                generator produces here, or quiescence stood pat while in
//                check, or stood pat on a number other than evaluate()'s. On
//                a cleared table at the widest window none of these is
//                reachable; each is refused rather than assumed, which is
//                what lets the quiet outcome mean exactly what the test
//                asserts. A PV move the generator does not produce here is
//                the same refusal.
//
// A promotion that takes nothing is in quiescence's move set when it is to a
// queen (S131), so a position whose best quiescence move queens a pawn is not
// a leaf and the walk plays the promotion like a capture. The three
// underpromotions that take nothing are not in that set and do not make a
// position unquiet.
//
// What the walk does to the game it is handed: every move it makes is unmade
// before it returns, so the game is where it was, history included.

enum leaf_outcome_t : uint8_t
{
  LEAF_QUIET = 0,
  LEAF_TERMINAL,
  LEAF_TRUNCATED,
  LEAF_ABORTED,
  LEAF_UNCERTIFIED,
  LEAF_OUTCOME_COUNT
};

const char* leaf_outcome_name(leaf_outcome_t outcome);


// Nodes one quiescence call of the walk may search before it is refused as
// aborted. A safety bound and not a setting: quiescence trees from game
// positions are thousands of nodes, the busiest test positions stay under
// 100000 ("quiescence stays bounded" in tests/test_search.cpp), and a call
// that reaches ten times that has met a position the walk should not
// certify. Datagen counts every refusal, so a cap that binds says so.
inline constexpr uint64_t LEAF_WALK_NODE_CAP = 1000000;


struct leaf_walk_t
{
  leaf_outcome_t outcome = LEAF_UNCERTIFIED;

  // Plies from the position the walk started at to the one it stopped at, and
  // how many of them were the principal variation's (leaf_walk_pv() only).
  int distance = 0;
  int pv_plies = 0;

  // The position the walk stopped at, whatever the outcome: the leaf when it
  // is quiet, the position that refused it otherwise.
  std::string fen;
  int phase = 0;
};


// What a walk needs and nothing a game needs: a table of its own, cleared
// before every quiescence call, and a search state of its own, so the walk
// neither reads nor disturbs the game's search and is a function of the
// position alone. One per thread; not copyable.
struct leaf_walker_t
{
  leaf_walker_t();
  ~leaf_walker_t();

  leaf_walker_t(const leaf_walker_t&) = delete;
  leaf_walker_t& operator=(const leaf_walker_t&) = delete;

  // 0 is MAX_QSEARCH_DEPTH, read when a walk starts so the tune build's
  // setting is the one honoured. A test narrows it to separate a truncated
  // walk from a quiet one without depending on the parameter's value.
  int max_plies = 0;
  uint64_t node_cap = LEAF_WALK_NODE_CAP;

  // Every node every call of this walker searched, for datagen's cost line.
  uint64_t nodes = 0;

  transposition_table_t tt = {};
  std::unique_ptr<search_state_t> state;
  std::atomic_bool stop{false};
};

// The ply bound a walk by this walker uses.
int leaf_walk_ply_bound(const leaf_walker_t* walker);

// The quiescence leaf of the position `game` stands on. Reproducible from the
// FEN alone: the walk reads no table and no state the game carries.
leaf_walk_t leaf_walk_quiescence(leaf_walker_t* walker, game_t* game);

// The principal variation `pv` played out from the position `game` stands on,
// and its end resolved as leaf_walk_quiescence() resolves a position. The line
// is the caller's: datagen hands it the last completed iteration's, never an
// aborted one's. Not reproducible from the FEN alone, because the line is the
// search's.
leaf_walk_t leaf_walk_pv(leaf_walker_t* walker, game_t* game, const pv_t* pv);

// One step of the walk, exported so a test can hold it: quiescence() at the
// widest window on the walker's table, cleared first. True with the value it
// returned and the move its root's exact entry names -- 0 when it stood pat or
// had no legal move. False, with `aborted` saying which, when the call ran out
// of the node cap or its root left no exact entry.
bool leaf_quiescence_choice(leaf_walker_t* walker,
                            game_t* game,
                            int* value,
                            move_t* chosen,
                            bool* aborted);
