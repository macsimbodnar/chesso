#!/usr/bin/env python3
"""The SPRT ledger, regenerated from `git log` and the pre-DEC-220 seed.

    python3 tools/ledger.py              # the table and the figures paragraph
    python3 tools/ledger.py --table      # the table alone
    python3 tools/ledger.py --figures    # the figures paragraph alone
    python3 tools/ledger.py --no-git     # the seed rows alone, no git call
    python3 tools/ledger.py --audit-seed-class   # the seed's class column
                                                 # against the rule below

That first command is what regenerates `adocs/plan.md`'s "What this costs"
ledger: its output replaces the table under "### The ledger" and the figures
paragraph after it, whole, at every verdict (DEC-136, DEC-220). Nothing in
that section is typed by hand any more -- the eight "With X the ledger holds
N" paragraphs it replaces were re-typed at every verdict and are the class of
number the 2026-09-04 plan review found stale (its F03).

WHERE THE ROWS COME FROM. Two sources, in this order:

  1. `adocs/data/ledger_seed.tsv`, the twenty verdicts taken before DEC-220,
     copied once from the table they are printed back into and never
     rewritten. Its own header says so.
  2. Every commit in `git log` whose message carries DEC-220's result block,
     oldest first. A commit that closes an SPRT verdict -- H1, H0 or no
     verdict -- carries six lines after its body and before `Bench:`:

         SPRT | cand <sha> vs ref <sha>, <tc>, Hash=<n>, <book>, {e0, e1} nElo
         Elo | <x> +/- <y>, nElo <x> +/- <y>
         LLR | <l> (<a>, <b>) -> H1|H0|none
         Games | N: <n> W: <w> L: <l> D: <d>, Ptnml [<5>]
         Wall | <h> h <m> m, <g> games/h, forfeits <f>
         Log | adocs/data/<file>

     `tools/gate.sh` refuses a commit whose block is missing a line, malformed
     or disagrees with the `Results of cand-<sha> vs ref-<sha>` line of the log
     it names, so a block that reaches `git log` has already been checked
     against its own evidence. This script checks the shape again anyway and
     dies loudly on the first bad one: a ledger that silently drops a verdict
     is worse than one that refuses to print.

THE RUN ID AND THE DESCRIPTION are not in the block -- it carries shas, not
step ids -- so they are read from the commit subject, by this rule:

  run    the first `S<nnn>` in the subject, plus any run qualifiers that
         follow it immediately (` v<n>`, ` leg <n>`, ` F<nn>`), so
         "Record S098 v3 leg 1's H0 for ..." gives "S098 v3 leg 1". A subject
         with no `S<nnn>` is an error naming the sha.
  what   the subject with, in order, a leading "Record ", the run phrase, a
         possessive "'s", a leading verdict word ("H1", "H0", "no verdict",
         "verdict") with its punctuation and a leading "for " removed, then
         trimmed of surrounding punctuation. Nothing left means the whole
         subject is used.

So the subject a verdict-closing commit wants is "Record S231's H1 for the
null child's two-ply key" or "Record S231's verdict: the null child's two-ply
key" -- either reads out as run "S231", what "the null child's two-ply key".

THE CLASS RULE decides what a future verdict is expected to cost, which is the
only thing the split is for. plan.md states it as: an effect outside the
bounds interval is fast class; inside it, on a bound, or a true zero is slow.
An effect is known only through its interval, and "on a bound" and "a true
zero" are statements about the truth, so the coded rule compares intervals:

    fast   the nElo estimate's interval [x - y, x + y] is disjoint from the
           bounds pair [elo0, elo1]
    slow   the two overlap -- the truth may be inside the pair or on a bound

Both ends are nElo: the bounds pair is nElo under `model=normalized`, so the
`Elo |` line's nElo estimate is the one compared, never its Elo estimate. A
seed row's class is the seed's, not this rule's; the seed's header records the
two rows where they differ and `--audit-seed-class` prints the comparison.

CONVENTIONS. A duration prints truncated to the whole minute, which is how
plan.md's mean and median were written by hand; hours print to two decimals
and games an hour to one. Stdlib only, no build, no engine, runs in under a
second.
"""

import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SEED = os.path.join(REPO, "adocs", "data", "ledger_seed.tsv")

COLUMNS = ("run", "what", "wall", "games", "bounds", "verdict", "elo", "nelo",
           "class", "source")

# The six lines of DEC-220's block, one tight pattern each. Anchored, because a
# line that nearly matches is the failure this is written to catch.
BLOCK_RE = {
    "SPRT": re.compile(
        r"^SPRT \| cand (?P<cand>[0-9a-f]{7,40}) vs ref (?P<ref>[0-9a-f]{7,40}), "
        r"(?P<tc>[^,]+), Hash=(?P<hash>\d+), (?P<book>[^,]+), "
        r"\{(?P<elo0>-?\d+(?:\.\d+)?), (?P<elo1>-?\d+(?:\.\d+)?)\} nElo$"),
    "Elo": re.compile(
        r"^Elo \| (?P<elo>[-+]?\d+(?:\.\d+)?) \+/- (?P<elo_err>\d+(?:\.\d+)?), "
        r"nElo (?P<nelo>[-+]?\d+(?:\.\d+)?) \+/- (?P<nelo_err>\d+(?:\.\d+)?)$"),
    "LLR": re.compile(
        r"^LLR \| (?P<llr>[-+]?\d+(?:\.\d+)?) "
        r"\((?P<a>[-+]?\d+(?:\.\d+)?), (?P<b>[-+]?\d+(?:\.\d+)?)\) "
        r"-> (?P<outcome>H1|H0|none)$"),
    "Games": re.compile(
        r"^Games \| N: (?P<n>\d+) W: (?P<w>\d+) L: (?P<l>\d+) D: (?P<d>\d+), "
        r"Ptnml \[(?P<ptnml>\d+, \d+, \d+, \d+, \d+)\]$"),
    "Wall": re.compile(
        r"^Wall \| (?P<h>\d+) h (?P<m>\d+) m, (?P<rate>\d+(?:\.\d+)?) games/h, "
        r"forfeits (?P<forfeits>\d+)$"),
    "Log": re.compile(r"^Log \| (?P<path>adocs/data/[A-Za-z0-9._/-]+)$"),
}

RUN_RE = re.compile(r"\bS\d{3}((?: v\d+| leg \d+| F\d+)*)")
NUMBER_WORDS = (
    "zero one two three four five six seven eight nine ten eleven twelve "
    "thirteen fourteen fifteen sixteen seventeen eighteen nineteen twenty "
    "twenty-one twenty-two twenty-three twenty-four twenty-five twenty-six "
    "twenty-seven twenty-eight twenty-nine thirty").split()


class LedgerError(Exception):
    """A row that cannot be parsed. Loud by design: never a dropped verdict."""


def die(message):
    sys.stderr.write("ledger.py: %s\n" % message)
    sys.exit(2)


def wall_seconds(text):
    """Seconds from a `<h> h <m> m <s> s` wall string, any part absent."""
    total = 0
    for value, unit in re.findall(r"(\d+)\s*([hms])\b", text):
        total += int(value) * {"h": 3600, "m": 60, "s": 1}[unit]
    if total == 0:
        raise LedgerError("wall time [%s] parses to nothing" % text)
    return total


def hm(seconds):
    """Truncated to the whole minute: plan.md's own convention for these."""
    seconds = int(seconds)
    return "%d h %02d m" % (seconds // 3600, seconds % 3600 // 60)


def parse_bounds(text):
    match = re.match(r"^\{(-?\d+(?:\.\d+)?), (-?\d+(?:\.\d+)?)\}$", text.strip("`"))
    if not match:
        raise LedgerError("bounds [%s] are not in `{elo0, elo1}` form" % text)
    return float(match.group(1)), float(match.group(2))


def classify(nelo, nelo_err, elo0, elo1):
    """fast when the nElo interval misses the bounds pair, slow when it does not."""
    low, high = nelo - nelo_err, nelo + nelo_err
    return "fast" if (high < elo0 or low > elo1) else "slow"


def parse_nelo(text):
    match = re.match(r"^([-+]?\d+(?:\.\d+)?) \+/- (\d+(?:\.\d+)?)$", text.strip())
    if not match:
        raise LedgerError("nElo [%s] is not in `<x> +/- <y>` form" % text)
    return float(match.group(1)), float(match.group(2))


def read_seed(path):
    rows = []
    with open(path) as handle:
        header = None
        for number, line in enumerate(handle, 1):
            if line.startswith("#") or not line.strip():
                continue
            cells = line.rstrip("\n").split("\t")
            if header is None:
                header = cells
                if tuple(header) != COLUMNS:
                    die("%s:%d: header is %s, expected %s"
                        % (path, number, header, list(COLUMNS)))
                continue
            if len(cells) != len(COLUMNS):
                die("%s:%d: %d columns, expected %d"
                    % (path, number, len(cells), len(COLUMNS)))
            row = dict(zip(COLUMNS, cells))
            try:
                row["seconds"] = wall_seconds(row["wall"])
                row["games_n"] = int(row["games"])
                parse_bounds(row["bounds"])
                parse_nelo(row["nelo"])
            except (LedgerError, ValueError) as error:
                die("%s:%d: %s" % (path, number, error))
            if row["class"] not in ("fast", "slow"):
                die("%s:%d: class is [%s], expected fast or slow"
                    % (path, number, row["class"]))
            row["origin"] = "%s:%d" % (os.path.relpath(path, REPO), number)
            rows.append(row)
    if header is None:
        die("%s: no header row" % path)
    return rows


def split_run_and_what(subject):
    """The run id and the description, from the commit subject. See the header."""
    match = RUN_RE.search(subject)
    if not match:
        raise LedgerError("subject [%s] carries no S<nnn> run id; a commit "
                          "closing a verdict names its step in the subject"
                          % subject)
    run = match.group(0)
    what = subject[:match.start()] + subject[match.end():]
    what = re.sub(r"^Record\s+", "", what.strip())
    what = re.sub(r"^'s\b", "", what).strip()
    what = re.sub(r"^(?:no verdict|verdict|H1|H0)\b[:,]?\s*", "", what)
    what = re.sub(r"^(?:no verdict|verdict)\b[:,]?\s*", "", what)
    what = re.sub(r"^for\s+", "", what)
    what = what.strip(" \t:,;-")
    return run, what or subject.strip()


def parse_block(sha, subject, body):
    """One commit's six lines into a row, or a LedgerError naming what is wrong."""
    found = {}
    for line in body.split("\n"):
        name = line.split(" |", 1)[0]
        if name not in BLOCK_RE or " |" not in line:
            continue
        match = BLOCK_RE[name].match(line.rstrip())
        if not match:
            raise LedgerError("%s: the `%s |` line does not parse:\n    %s"
                              % (sha, name, line.rstrip()))
        if name in found:
            raise LedgerError("%s: two `%s |` lines; exactly one is allowed"
                              % (sha, name))
        found[name] = match
    missing = [name for name in ("SPRT", "Elo", "LLR", "Games", "Wall", "Log")
               if name not in found]
    if missing:
        raise LedgerError("%s: the result block is missing %s"
                          % (sha, ", ".join("`%s |`" % name for name in missing)))

    sprt, elo, llr, games, wall = (found["SPRT"], found["Elo"], found["LLR"],
                                   found["Games"], found["Wall"])
    elo0, elo1 = float(sprt.group("elo0")), float(sprt.group("elo1"))
    nelo = float(elo.group("nelo"))
    nelo_err = float(elo.group("nelo_err"))
    outcome = llr.group("outcome")
    estimate = "%+.2f +/- %s" % (float(elo.group("elo")), elo.group("elo_err"))
    run, what = split_run_and_what(subject)

    return {
        "run": run,
        "what": what,
        "wall": "%s h %s m" % (wall.group("h"), wall.group("m")),
        "games": games.group("n"),
        "bounds": "{%s, %s}" % (sprt.group("elo0"), sprt.group("elo1")),
        "verdict": ("**no verdict**" if outcome == "none"
                    else "%s, %s" % (outcome, estimate)),
        "elo": estimate,
        "nelo": "%s +/- %s" % (elo.group("nelo"), elo.group("nelo_err")),
        "class": classify(nelo, nelo_err, elo0, elo1),
        "source": found["Log"].group("path"),
        "seconds": int(wall.group("h")) * 3600 + int(wall.group("m")) * 60,
        "games_n": int(games.group("n")),
        "origin": "commit %s" % sha,
    }


def read_commits():
    """Every commit carrying an `SPRT |` line, oldest first."""
    separator = "\x1e"
    try:
        log = subprocess.check_output(
            ["git", "-C", REPO, "log", "--reverse",
             "--format=%s" % (separator + "%h%n%s%n%B")],
            universal_newlines=True)
    except (subprocess.CalledProcessError, OSError) as error:
        die("git log failed: %s" % error)

    rows = []
    for chunk in log.split(separator):
        if not chunk.strip():
            continue
        sha, subject, body = (chunk.lstrip("\n").split("\n", 2) + ["", ""])[:3]
        if not re.search(r"^SPRT \|", body, re.M):
            continue
        try:
            rows.append(parse_block(sha, subject, body))
        except LedgerError as error:
            die(str(error))
    return rows


def figures(rows):
    seconds = sorted(row["seconds"] for row in rows)
    total_seconds = sum(seconds)
    total_games = sum(row["games_n"] for row in rows)
    count = len(seconds)
    middle = (seconds[count // 2] if count % 2
              else (seconds[count // 2 - 1] + seconds[count // 2]) / 2.0)
    by_class = {}
    for name in ("fast", "slow"):
        members = [row["seconds"] for row in rows if row["class"] == name]
        by_class[name] = (len(members),
                          sum(members) / float(len(members)) if members else 0)
    return {
        "count": count,
        "mean": total_seconds / float(count),
        "median": middle,
        "games": total_games,
        "hours": total_seconds / 3600.0,
        "rate": total_games / (total_seconds / 3600.0),
        "fast": by_class["fast"],
        "slow": by_class["slow"],
    }


def word(number):
    return NUMBER_WORDS[number] if number < len(NUMBER_WORDS) else str(number)


def render_table(rows):
    out = ["| run | what it measured | wall | games | bounds | verdict |",
           "|---|---|---|---|---|---|"]
    for row in rows:
        out.append("| %s | %s | %s | %s | `%s` | %s |"
                   % (row["run"], row["what"], row["wall"], row["games"],
                      row["bounds"], row["verdict"]))
    return "\n".join(out)


def render_figures(rows):
    f = figures(rows)
    return ("**The ledger holds %s: mean %s, median %s, %d games in %.2f hours, "
            "%.1f an hour across the set.** **Fast class**, an effect outside "
            "the bounds interval -- %s runs, mean **%s**. **Slow class**, "
            "inside it, on a bound or a true zero -- %s runs, mean **%s**."
            % (word(f["count"]), hm(f["mean"]), hm(f["median"]), f["games"],
               f["hours"], f["rate"],
               word(f["fast"][0]), hm(f["fast"][1]),
               word(f["slow"][0]), hm(f["slow"][1])))


def audit_seed_class(rows):
    """The seed's recorded class against what the rule would say. Evidence, not a gate."""
    out = ["%-14s %-17s %-6s %-6s %s"
           % ("run", "nelo", "seed", "rule", "")]
    for row in rows:
        nelo, nelo_err = parse_nelo(row["nelo"])
        elo0, elo1 = parse_bounds(row["bounds"])
        ruled = classify(nelo, nelo_err, elo0, elo1)
        out.append("%-14s %-17s %-6s %-6s %s"
                   % (row["run"], row["nelo"], row["class"], ruled,
                      "" if ruled == row["class"] else "DISAGREES"))
    return "\n".join(out)


def main(argv):
    want = set(argv[1:])
    unknown = want - {"--table", "--figures", "--no-git", "--audit-seed-class"}
    if unknown:
        die("unknown option(s) %s; see the header for the four"
            % ", ".join(sorted(unknown)))

    rows = read_seed(SEED)
    if "--no-git" not in want and "--audit-seed-class" not in want:
        rows = rows + read_commits()

    if "--audit-seed-class" in want:
        print(audit_seed_class(rows))
        return 0
    if "--table" in want:
        print(render_table(rows))
        return 0
    if "--figures" in want:
        print(render_figures(rows))
        return 0
    print(render_table(rows))
    print("")
    print(render_figures(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
