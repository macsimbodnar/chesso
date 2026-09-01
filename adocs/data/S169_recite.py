#!/usr/bin/env python3
"""S169. Re-anchor the stale citations in the pending step files, once.

    adocs/data/S169_recite.py propose            # print what it would change
    adocs/data/S169_recite.py detail             # what the script cannot decide
    adocs/data/S169_recite.py apply              # rewrite them, and write the mapping

**This is a generator, not a gate, and re-running it later proves nothing.**
`tools/plan_prose_check.py` reads a step file's DRIFT baseline as the commit
that last wrote it, so the moment this script rewrites a file that file's
baseline becomes this step's own commit and every citation in it is DRIFT-green
by construction. That is why the evidence is a tracked mapping written while the
old baselines are still reachable, and not the checker's verdict. DEC-119.

Two relocation methods, tried in this order per flagged citation:

  BLOCK  the exact text the cited range held at the step file's baseline commit
         is found, once, somewhere in the file as it stands now. The repair is
         that location. This is an exact string match on the whole block, so a
         citation repaired this way points at the same text it was written for
         and the mapping records both sides.
  TITLE  the block is gone or is no longer unique, and a doctest title quoted
         beside the citation names a test that exists. The repair is where that
         title opens now, keeping the span. Weaker than BLOCK -- the body moved
         under the title -- so the mapping records the two texts as different
         and says so, and `tools/plan_prose_check.py`'s ANCHOR check gates the
         result afterwards without consulting any baseline.

Anything neither method resolves is printed as UNRESOLVED with its sentence and
is repaired by hand. Nothing is repaired by deleting a citation, by dropping a
line number, or by widening a range until the checker stops complaining.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(REPO, "tools"))

import plan_prose_check as ppc  # noqa: E402


# The nineteen neither method could decide, each resolved by reading the tree
# and the citing sentence. Keyed by (step file, line in it, the citation as
# written); the value is (repaired citation, why). A value equal to its key's
# citation is a HOLD: the citation is right and the flag is a cosmetic edit to
# the cited text, which is a thing DRIFT cannot tell from a real move.
#
# Everything here was resolved against src/search.cpp at HEAD, where
# quiescence() opens at 293 and negamax() at 589 -- which is what separates the
# two `state->explored_nodes++` and the two `tt_get_entry` calls the file now
# has.
HAND = {
    ("adocs/plan_todo/S020_single_check_computation.md", 44,
     "src/search.cpp:469"): (
        "src/search.cpp:687",
        "negamax's `const bool is_in_check = is_check(game);`. Line 469 was "
        "blank at the baseline, so this citation was wrong before it drifted."),
    ("adocs/plan_todo/S082_datagen_qsearch_leaf_labels.md", 29,
     "src/search.cpp:125"): (
        "src/search.cpp:349",
        "the evaluate_lazy() call in quiescence, which is what the sentence "
        "says the line does. Line 125 held a bare `}` at the baseline."),
    ("adocs/plan_todo/S082_datagen_qsearch_leaf_labels.md", 48,
     "src/search.cpp:29"): (
        "src/search.cpp:27",
        "the same comment block, reflowed: line 27 is the line that names "
        "MAX_QSEARCH_DEPTH and says it lives in search_params.hpp. The "
        "sentence's claim that the value is 8 is not this step's to touch -- "
        "src/search_params.hpp:106 compiles 19."),
    ("adocs/plan_todo/S095_internal_iterative_reduction.md", 82,
     "src/search.cpp:455"): (
        "src/search.cpp:663",
        "negamax's tt_get_entry, not quiescence's at 318: the sentence is "
        "about the main search's probe, which is what an internal iterative "
        "reduction reads."),
    ("adocs/plan_todo/S097_singular_extensions.md", 125,
     "src/data_structures.hpp:441-460"): (
        "src/data_structures.hpp:447-483",
        "struct search_state_t whole, which is what the sentence cites it "
        "for. It opens at 447 and closes at 483."),
    ("adocs/plan_todo/S097_singular_extensions.md", 138,
     "src/data_structures.hpp:388-414"): (
        "src/data_structures.hpp:388-420",
        "struct tt_entry_t whole. It still opens at 388 and now closes at "
        "420; the old range stopped six lines short of the closing brace."),
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 117,
     "src/search_params.hpp:29-32"): (
        "src/search_params.hpp:29-32",
        "HOLD. The comment is still there and still says what is cited; one "
        "word was dropped from its first line."),
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 166,
     "src/search.cpp:210"): (
        "src/search.cpp:302",
        "quiescence's `state->explored_nodes++`, which is the one the "
        "sentence means; negamax's is at 608."),
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 218,
     "search_params.hpp:29-32"): (
        "search_params.hpp:29-32",
        "HOLD, same comment as the citation at line 117 of this file."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 113,
     "tests/test_search.cpp:1926"): (
        "tests/test_search.cpp:2847",
        'where "pruning does not hide a mate against the material leader" '
        "opens now. The title is on the citing line but the pairing rule "
        "could not attach it, so this arrived as DRIFT rather than ANCHOR."),
    ("adocs/plan_todo/S120_eval_cache.md", 102,
     "src/search.cpp:257"): (
        "src/search.cpp:349",
        "the evaluate_lazy() call in quiescence, which is what the sentence "
        "cites."),
    ("adocs/plan_todo/S120_eval_cache.md", 206,
     "src/chesso.cpp:1128"): (
        "src/chesso.cpp:1155",
        "the tt_reset inside command_ucinewgame, which is what the sentence "
        "says; the other at 1618 is command_clean_TT."),
    ("adocs/plan_todo/S120_eval_cache.md", 230,
     "src/search.cpp:251-257"): (
        "src/search.cpp:343-349",
        "the same statement pair, renamed: `stand_pat_is_exact` became "
        "`static_eval_is_exact` and `stand_pat` became `static_eval` at "
        "S130, so no exact block survives to relocate."),
    ("adocs/plan_todo/S131_quiescence_promotions.md", 116,
     "search.cpp:210"): (
        "search.cpp:302",
        "quiescence's `state->explored_nodes++`. The bare basename spelling "
        "is the file's own and is left as it is; giving it a path is S144's."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 87,
     "src/data_structures.hpp:447"): (
        "src/data_structures.hpp:453",
        "search_state_t::explored_nodes. The identical declaration at 440 "
        "belongs to another struct, and 447 is now the `struct "
        "search_state_t` line itself."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 88,
     "src/search.cpp:430"): (
        "src/search.cpp:608",
        "negamax's `state->explored_nodes++`, which is the one the sentence "
        "calls negamax entry."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 88,
     "src/search.cpp:210"): (
        "src/search.cpp:302",
        "quiescence's `state->explored_nodes++`, the other half of the same "
        "sentence."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 92,
     "src/search.cpp:645-793"): (
        "src/search.cpp:896-1063",
        "negamax's shared move loop, `for (size_t i = 0;; ++i)`, which opens "
        "at 896 and closes at 1063."),
    ("adocs/plan_todo/S159_killer_slot_ageing.md", 5,
     "src/search_params.hpp:29"): (
        "src/search_params.hpp:29",
        "HOLD. Same comment line, one word dropped from it."),
}

def flagged_citations():
    """Every gated citation in the pending step files that the checker flags.

    Same walk as ppc.check_citations, but it yields the citation and the
    baseline rather than printing a line about it.
    """
    tracked, byname = ppc._tracked()
    out = []
    for d in ("plan_todo", "plan_current"):
        base = os.path.join(REPO, "adocs", d)
        if not os.path.isdir(base):
            continue
        for name in sorted(os.listdir(base)):
            if not name.endswith(".md"):
                continue
            path = os.path.join(base, name)
            rel = os.path.relpath(path, REPO)
            rev = ppc.baseline(rel)
            text = open(path, encoding="utf-8").read()
            for first, para in ppc.paragraphs(text):
                for line, spelled, resolved, a, b, _kind in ppc.cites_in(
                        para, first, tracked, byname):
                    if spelled is None or resolved is None:
                        continue
                    if not ppc.gated(resolved):
                        continue
                    now = ppc._at(None, resolved)
                    if now is None or a < 1 or a > b or b > len(now):
                        continue
                    cite = f"{spelled}:{a}" + (f"-{b}" if b != a else "")
                    claimed = ppc.anchor_titles(para, line, first, resolved,
                                                cite)
                    titles = ppc.titles_of(resolved)
                    kind = None
                    if claimed and not any(a <= titles[t] <= b
                                           for t in claimed):
                        kind = "ANCHOR"
                    elif rev is not None:
                        then = ppc._at(rev, resolved)
                        if then is not None and b <= len(then):
                            if "\n".join(then[a - 1:b]) != "\n".join(
                                    now[a - 1:b]):
                                kind = "DRIFT"
                    if kind:
                        out.append(dict(file=rel, line=line, rev=rev,
                                        spelled=spelled, resolved=resolved,
                                        a=a, b=b, cite=cite, kind=kind,
                                        claimed=claimed, para=para,
                                        first=first))
    return out


def find_block(now, block):
    """Every start index (1-based) where `block` sits in `now`, exactly."""
    n = len(block)
    return [i + 1 for i in range(len(now) - n + 1) if now[i:i + n] == block]


def relocate(c):
    """(method, new_a, new_b, why) for one flagged citation, or None."""
    key = (c["file"], c["line"], c["cite"])
    if key in HAND:
        cite, why = HAND[key]
        a, _, rest = cite.partition(":")
        lo, _, hi = rest.partition("-")
        lo = int(lo)
        hi = int(hi) if hi else lo
        return ("HOLD" if cite == c["cite"] else "HAND", lo, hi, why)
    now = ppc._at(None, c["resolved"])
    then = ppc._at(c["rev"], c["resolved"]) if c["rev"] else None
    span = c["b"] - c["a"]
    why = "no baseline text"
    if then is not None and c["b"] <= len(then):
        block = then[c["a"] - 1:c["b"]]
        if not any(ln.strip() for ln in block):
            why = "cited range is blank at the baseline"
        else:
            hits = find_block(now, block)
            if len(hits) == 1:
                return ("BLOCK", hits[0], hits[0] + span, "exact block, unique")
            why = (f"block occurs {len(hits)} times at HEAD" if hits
                   else "block gone from the file")
    # BLOCK could not decide. A title beside the citation is the other stable
    # reference, and it is the right one for an ANCHOR flag whichever way BLOCK
    # went: the test moved and the title is what names it.
    titles = ppc.titles_of(c["resolved"])
    live = [t for t in c["claimed"] if t in titles]
    if len(live) == 1:
        t = titles[live[0]]
        return ("TITLE", t, t + span, f'title "{live[0]}" opens at {t}')
    if len(live) > 1:
        return (None, None, None, f"{why}; {len(live)} titles claimed")
    return (None, None, None, f"{why}; no live title beside it")


def sentence_of(c):
    """The citing line, flattened, for a human to read in a report."""
    lines = c["para"].split("\n")
    i = c["line"] - c["first"]
    return " ".join(" ".join(lines[max(0, i - 1):i + 2]).split())[:220]


def proposals():
    out = []
    for c in flagged_citations():
        method, a, b, why = relocate(c)
        c.update(method=method, new_a=a, new_b=b, why=why)
        out.append(c)
    return out


def new_cite(c):
    return f"{c['spelled']}:{c['new_a']}" + (
        f"-{c['new_b']}" if c["new_b"] != c["new_a"] else "")


def do_propose():
    props = proposals()
    ok = [p for p in props if p["method"]]
    bad = [p for p in props if not p["method"]]
    for p in props:
        if p["method"]:
            print(f"{p['method']:6} {p['file']}:{p['line']}  {p['cite']}"
                  f"  ->  {new_cite(p)}   ({p['kind']}, {p['why']})")
    for p in bad:
        print(f"UNRESOLVED {p['file']}:{p['line']}  {p['cite']}"
              f"   ({p['kind']}, {p['why']})")
        print(f"           {sentence_of(p)}")
    tally = ", ".join(
        f"{sum(1 for p in ok if p['method'] == m)} {m}"
        for m in ("BLOCK", "TITLE", "HAND", "HOLD"))
    print(f"\n{len(props)} flagged, {len(ok)} resolved ({tally}), "
          f"{len(bad)} unresolved")
    return 0 if not bad else 1


def do_detail():
    """Everything a human needs to resolve one citation the script could not.

    The baseline text, the candidate locations of that text at HEAD, and the
    sentence doing the citing. Run before `apply`, while the baselines are
    still reachable.
    """
    for c in proposals():
        if c["method"]:
            continue
        then = ppc._at(c["rev"], c["resolved"])
        now = ppc._at(None, c["resolved"])
        block = then[c["a"] - 1:c["b"]] if then and c["b"] <= len(then) else []
        print(f"--- {c['file']}:{c['line']}  {c['cite']}  ({c['kind']}, "
              f"{c['why']})")
        print(f"    sentence: {sentence_of(c)}")
        print(f"    baseline {c['rev'][:7]} held:")
        for k, ln in enumerate(block[:8]):
            print(f"      {c['a'] + k:6}| {ln[:110]}")
        hits = find_block(now, block) if block else []
        print(f"    at HEAD that text is at: {hits[:12]}")
        print(f"    HEAD now holds at {c['a']}-{c['b']}:")
        for k in range(c["a"], min(c["b"], c["a"] + 7) + 1):
            print(f"      {k:6}| {now[k - 1][:110]}")
    return 0


def do_apply():
    """Rewrite the citations, then write the mapping beside this script.

    Both in one run and in this order, because `apply` dirties the step files
    and `plan_prose_check.baseline()` returns None for a dirty file -- so a
    second invocation could not recompute what the first one repaired.
    """
    props = _staged_proposals()
    by_file = {}
    for p in props:
        by_file.setdefault(p["file"], []).append(p)
    changed = held = 0
    for rel, group in sorted(by_file.items()):
        full = os.path.join(REPO, rel)
        lines = open(full, encoding="utf-8").read().split("\n")
        for p in sorted(group, key=lambda q: -q["line"]):
            if p["method"] == "HOLD":
                held += 1
                continue
            i = p["line"] - 1
            old, new = p["cite"], new_cite(p)
            # Bounded on the right the way plan_prose_check's own DIRECT is, so
            # `src/search.cpp:210` is not counted inside `src/search.cpp:2101`.
            pat = re.compile(re.escape(old) + r"(?![\d\w])(?!\.\d)")
            hits = pat.findall(lines[i])
            if len(hits) != 1:
                print(f"SKIP {rel}:{p['line']} {old} appears {len(hits)} "
                      f"times on its line")
                continue
            lines[i] = pat.sub(new, lines[i], count=1)
            changed += 1
        open(full, "w", encoding="utf-8").write("\n".join(lines))
    print(f"{changed} citations rewritten, {held} held, "
          f"in {len(by_file)} step files")
    with open(os.path.join(HERE, "S169_recitations.tsv"), "w",
              encoding="utf-8") as fh:
        fh.write("\t".join(COLUMNS) + "\n")
        for row in evidence_rows():
            fh.write("\t".join(str(x) for x in row) + "\n")
    print(f"mapping written to adocs/data/S169_recitations.tsv "
          f"({len(props)} rows)")
    return 0


COLUMNS = ("step_file", "step_line", "baseline", "cited_file", "old_range",
           "new_range", "method", "flag", "texts_equal", "baseline_text",
           "head_text", "why")


def evidence_rows():
    """One row per repaired citation: what it pointed at, and what it points at.

    `baseline_text` is what the cited range held in the commit the step file
    was last written in -- the text the citation was written for. `head_text`
    is what the new range holds in the working tree. BLOCK repairs have them
    equal by construction, which is the whole of their proof; TITLE, HAND and
    HOLD repairs do not, and their `why` says what was read instead.
    """
    rows = []
    for p in _staged_proposals():
        then = ppc._at(p["rev"], p["resolved"])
        now = ppc._at(None, p["resolved"])
        old_text = ("\n".join(then[p["a"] - 1:p["b"]])
                    if then and p["b"] <= len(then) else "")
        new_text = "\n".join(now[p["new_a"] - 1:p["new_b"]])
        rows.append((p["file"], p["line"], p["rev"][:7], p["resolved"],
                     f"{p['a']}-{p['b']}", f"{p['new_a']}-{p['new_b']}",
                     p["method"], p["kind"],
                     "yes" if old_text == new_text else "no",
                     _flat(old_text), _flat(new_text), _flat(p["why"])))
    return rows


def _flat(s):
    return " ".join(s.split())[:200]


_CACHE = []


def _staged_proposals():
    """The proposals, computed once and remembered across apply/evidence."""
    if not _CACHE:
        _CACHE.extend(p for p in proposals() if p["method"])
    return _CACHE


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else "propose"
    if mode == "propose":
        sys.exit(do_propose())
    if mode == "detail":
        sys.exit(do_detail())
    if mode == "apply":
        sys.exit(do_apply())
    print(__doc__)
    sys.exit(2)
