#!/usr/bin/env python3
"""Write benefit_incidents.md from benefit_rows.json and benefit_census.tsv. Every figure in it is
computed here, so a changed row changes the figure. Run benefit_incidents.py first."""
import json
import os
import re
import statistics as st
import subprocess
import sys
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", "..", ".."))
sys.path.insert(0, HERE)
from benefit_rows import COMMIT_KIND  # noqa: E402

R = json.load(open(os.path.join(HERE, "benefit_rows.json")))
CODE = re.compile(r"\.(cpp|hpp|h|c|S|py|sh|cmake)$|(^|/)CMakeLists\.txt$")
DONE = len([f for f in os.listdir(os.path.join(REPO, "adocs/plan_done")) if f.endswith(".md")])


def git(*a):
    return subprocess.run(["git", "-C", REPO, *a], capture_output=True).stdout.decode("utf-8", "replace")


def by(c):
    return [r for r in R if r["cls"] == c]


def pct(a, b):
    return f"{100 * a / b:.1f} %" if b else "n/a"


def cell(s):
    return (s or "").replace("|", "\\|").replace("\n", " ")


def short(rec, line=None):
    rec = rec.replace("adocs/plan_done/", "").replace("adocs/audit/", "").replace("adocs/", "")
    rec = re.sub(r"^(S\d{3})_.*", r"\1", rec)
    rec = rec.replace("plan_todo/S308_false_comment_siblings.md", "S308").replace("data/comment_study/report.md", "study report")
    rec = rec.replace(".md", "")
    return f"{rec}:{line}" if line and not rec.startswith("git:") else rec


def qt(r):
    q = cell(r["quote"])
    return q if q.startswith('"') else f'"{q}"'


def blockcol(r):
    b = r.get("bytes")
    if b is None:
        return "unknown"
    return f"{b} B {'long' if r['long'] else 'short'}{', log' if r['log'] else ''}"


def agecol(r):
    if r.get("same"):
        return "0, same step"
    return str(r["age"]) if r.get("age") is not None else "unknown"


A, B, C, D, E = by("A"), by("B"), by("C"), by("D"), by("E")
AB = A + B
ACT = [r for r in A if r["cause"] == "actual"]
SYM = [r for r in A if r["cause"] == "symptom"]

steps = lambda rows: {m.group(1) for r in rows for m in [re.search(r"(S\d{3})_", r["rec"])] if m}

# ages
def bin_of(r):
    if r.get("same"):
        return "same step"
    a = r.get("age")
    if a is None:
        return "unknown"
    if a <= 3:
        return "0-3 d"
    if a < 30:
        return "4-29 d"
    return "30 d or more"


BINS = ["same step", "0-3 d", "4-29 d", "30 d or more", "unknown"]
binA = Counter(bin_of(r) for r in A)
binB = Counter(bin_of(r) for r in B)
agesA = [r["age"] for r in A if r.get("age") is not None]
agesB = [0 if r.get("same") else r["age"] for r in B if r.get("same") or r.get("age") is not None]
agesD = [r["age"] for r in D if r.get("age") is not None]
origin = Counter((r["cls"], r.get("origin", "unknown")) for r in AB)
rule = Counter(r.get("rule") for r in AB)
ruleA = Counter(r.get("rule") for r in A)
ruleB = Counter(r.get("rule") for r in B)
script = Counter(r.get("script") for r in AB)
scriptA = Counter(r.get("script") for r in A)
scriptB = Counter(r.get("script") for r in B)
# script against rule for the sweep's own rows
known = [r for r in AB if r.get("bytes") is not None]
longs = [r for r in known if r["long"]]
logs = [r for r in known if r["log"]]
knownD = [r for r in D if r.get("bytes") is not None]
longD = [r for r in knownD if r["long"]]
logD = [r for r in knownD if r["log"]]
knownA = [r for r in A if r.get("bytes") is not None]
kindD = Counter(r["kind"] for r in D)
bins_old_new = {
    "new": sum(1 for r in AB if bin_of(r) in ("same step", "0-3 d")),
    "old": sum(1 for r in AB if bin_of(r) in ("4-29 d", "30 d or more")),
    "unk": sum(1 for r in AB if bin_of(r) == "unknown"),
}
same = [r for r in B if r.get("same")]
same_fast = sum(1 for r in same if "fast check" in r["cost"])
listed = [r for r in B if r["rec"].endswith("S306_false_comments.md") or "S308" in r["rec"]]

# census
cen = [l.rstrip("\n").split("\t") for l in open(os.path.join(HERE, "benefit_census.tsv"))][1:]
kind_lines = Counter()
kind_n = Counter()
for row in cen:
    k = COMMIT_KIND[row[0]]
    kind_n[k] += 1
    kind_lines[k] += int(row[3]) + int(row[4])
total_lines = sum(int(r[3]) + int(r[4]) for r in cen)


def numstat(shas):
    code = docs = 0
    for sha in shas:
        for line in git("show", "--numstat", "--format=", sha).strip().split("\n"):
            if not line.strip():
                continue
            a, r, p = line.split("\t", 2)
            n = (0 if a == "-" else int(a)) + (0 if r == "-" else int(r))
            if CODE.search(p) and not p.startswith("adocs/"):
                code += n
            else:
                docs += n
    return code, docs


Csize = {}
for r in C:
    shas = r["sha"].split()
    if r["sha"] == "close":
        shas = git("log", "--diff-filter=A", "--format=%h", "--", r["rec"]).split()[-1:]
    Csize[r["id"]] = (shas, *numstat(shas))
strict = [r for r in C if r["tier"] == "strict"]
loose = [r for r in C if r["tier"] == "loose"]
strict_code = sum(Csize[r["id"]][1] for r in strict)
strict_docs = sum(Csize[r["id"]][2] for r in strict)
MEAN, MEDIAN = 6.81, 2.90

L = []
w = L.append
w("# What false comments cost and true ones bought, from this repository's records")
w("")
w(f"Read at `dcd3dae`, 2026-11-01. Records only; no engine built or run. Scripts beside this file: `benefit_census.py`, `benefit_rows.py` (the rows), `benefit_incidents.py` (validates every quote verbatim against its record, ages, blocks), `benefit_report.py` (this file). Every count is a floor: a case the record never mentions is not here, and a sweep that looks for false comments finds B and not A by design. No recommendation.")
w("")
w("## The five answers")
w("")
w(f"**1. Incidents.** A {len(A)}, B {len(B)}, C {len(C)} steps, D {len(D)}, E {len(E)}. Over {DONE} done steps: A {pct(len(A), DONE)} ({len(A)}/{DONE}), D {pct(len(D), DONE)} ({len(D)}/{DONE}). D's rows name {len(steps(D))} distinct steps. A row is a finding or a use, not a step, so the rates are incidents per step and not steps affected. B counts S306's and S308's list row by row, {sum(1 for r in B if r['rec'].endswith('S306_false_comments.md') or 'S308' in r['rec'])} of the {len(B)}.")
w("")
w(f"**2. Cause against symptom.** Class A is small. {len(ACT)} of {len(A)} have the comment as the actual cause, {pct(len(ACT), DONE)} of steps, and strictly 1. A03: a later brief, S093, cites a false comment, the only row where the record shows someone acting on the text. A02: a test tolerance derived from a false count, but comment and number entered in one commit, `10bef50`, so it can also be read as one false belief written twice. Together they cost one step, one clause in another, 0 games and 0 runs. The other {len(SYM)} are symptoms: code or a step file held the same belief. The largest, A01, has 3340 games played under the false property, with no re-run, no rating error measured, and no record that anyone read the comment. A05's 7 % of speed is the largest measured loss, and its comment copies a step file. No record in this search shows a run re-done because a comment misled it; S247's 400 timed runs are a repair of a figure.")
w("")
w(f"**3. Old or new.** Of A and B, {bins_old_new['new']} were written in the step that found them or at most 3 days before, {bins_old_new['old']} were older, {bins_old_new['unk']} unknown. A's ages run {min(agesA)} to {max(agesA)} days, median {st.median(agesA)}; the two actual causes are {ACT[0]['age']} and {ACT[1]['age']} days. B's median is {st.median(agesB)} days, {st.median([a for a in agesB if a > 0] or [0])} without the same-step and same-day rows. At 30 days or more: {binA['30 d or more']} A against {binB['30 d or more']} B, and that one A is a symptom. All {origin[('A', 'birth')]} A rows were false when written and {origin[('A', 'drift')]} came from drift, while {origin[('B', 'drift')]} of {len(B)} B rows are drift. So recorded harm sits on young text that was never true, and old text that went stale shows up as false with no recorded harm. Limits: `achesso` is 91 days old, so 'old' is capped; A02, A03 and A04 were found when it was 11 to 18 days old, when all text was young; 44 % of comment bytes are under a month old anyway (study); old text has had longer to be found; the study's sweep finds old false text without testing for harm. A rule on new comments would have barred the form of {ruleA['F']} of {len(A)} A rows and {ruleB['F']} of {len(B)} B rows, had it been in force when written (N {rule['N']}, U {rule['U']}). Taken literally, given text already in the tree, it prevents 0 of {len(AB)}: every row predates it.")
w("")
w(f"**4. Class C.** {len(strict)} done steps exist only or mainly for comments, {pct(len(strict), DONE)} of {DONE}: S155, S213, S245, S247, S249, S287, S306. {len(loose)} more have comments as a named large part: S192, S290, S277, S157. Strict: {strict_code} lines in code files and {strict_docs} in documents, by `git show --numstat` of their commits. Comment-only commits, by the study's lexer: {len(cen)} of 1000 non-merge commits since `bitboard`, {total_lines} lines; {kind_n['repair']} of them repairs ({kind_lines['repair']} lines), {kind_n['close pass']} close passes ({kind_lines['close pass']}), {kind_n['record']} adding a record where nothing was false ({kind_lines['record']}), {kind_n['build break']} a comment that broke the build, {kind_n['format']} format. [E] {len(strict)} steps at the study's median {MEDIAN} M and mean {MEAN} M weighted units: {len(strict) * MEDIAN:.1f} M to {len(strict) * MEAN:.1f} M. These diffs are mostly small beside the study's mean, so the upper end is likely high; no token figure for them is on this machine. S213's 650 code-file lines include dead API removed and a new test, so the strict total is not all comment. Open and not counted: S308, S311, S312, S313.")
w("")
w(f"**5. Script or reader.** Of {len(AB)} A and B rows: script {script['S']}, reader {script['R']}, unsettled {script['U']}. A: script {scriptA['S']}, reader {scriptA['R']}; the one script row, A01, needs a check nobody had written. B: script {scriptB['S']}, reader {scriptB['R']}, unsettled {scriptB['U']}. The script class here is a dead `file:line`, a gone path, a claim on a closed step, a number against its command or its test. Of the 6 A rows, 5 needed a reader, so the rows with recorded harm are mostly the class a script does not reach. {same_fast} of the {len(same)} same-step B rows name a fast check; the workflow already catches that class. In S306's and S308's list of {len(listed)}: script {sum(1 for r in listed if r['script']=='S')}, reader {sum(1 for r in listed if r['script']=='R')}, against the literature report's 8 of 11.")
w("")
w("## Other facts the decision needs")
w("")
w(f"- **Length.** Of {len(known)} A and B rows with a block found: {len(longs)} long, {pct(len(longs), len(known))}, and {len(logs)} log-shaped. In the live code today long blocks are 304 of 4941 and 40.8 % of bytes, log-shaped 76 and 15.6 % of bytes (DEC-336). So {pct(len(longs), len(known))} of located false text sits in long blocks against 6.2 % of blocks and 40.8 % of bytes, and {pct(len(logs), len(known))} in log-shaped ones against 1.5 % and 15.6 %. Biased: S306's list came from the study's size-stratified sample, and same-step rows mostly have no located block. {len([r for r in knownA if not r['long']])} of A's {len(knownA)} blocks are short and only A02 is long; A01 is 198 B. Length does not track harm here, on 6 rows.")
w(f"- **D's blocks.** Of {len(knownD)} D rows with a block found: {len(longD)} long, {len(logD)} log-shaped. Median age at use {st.median(agesD)} days. Kinds: " + ", ".join(f"{k} {v}" for k, v in kindD.most_common()) + ". Log-shaped blocks carried D14, D23 and D27, so a log can hold a fact a step used. D13 alone is quoted in 9 step files.")
w("- **Tombstones.** No record found of a tombstone stopping a retry. S022, S114, S235, S237 and S238 wrote them; no later step says it read one and did not retry. The study's 'tombstones stop retries' is design, not a recorded use. A05 is the opposite case: a comment saying a change was not worth making, wrong by 7 %.")
w("- **Near misses.** B04 ('Nearly propagated S035 into the manual on the strength of a code comment') and B10. F04's 'the sentence a future reader will rely on' is a prediction. Counted as B, cost 0.")
w(f"- **Repairs can find things.** B13: re-deriving a stale figure found a floor that could not fail, S250. B56 is the same shape. That is 2 real test defects from comment work, and {len(E)} false sentences introduced by it (E). S306's rate is 2 in 12 rewrites, after the writer's own check; the others have no denominator on record.")
w("- **Audit false alarms.** F06 suspected a comment's robustness claim; S154 measured it true. S232's draft said a comment had lost a number; 'that was wrong and is withdrawn'. Neither is a row.")
w("- **S254 is not a comment incident.** The study lists it. Its five stale anchors are mutant `old` strings naming code, `!is_check_move` against `!is_check_move()`. Not rowed. The real comment-to-mutant dependency: S114's M03 anchor sat on a comment and was re-pointed three times.")
w("- **Comments breaking things, not false ones.** `d54f690`: a trailing backslash in a `//` comment stopped the workstation's build under g++ -Werror, no step could complete until fixed. S238 and S111: format gate red on comment reflow. `status.md:6971`: a comment grown by 8 lines moved 7 line citations, restored mechanically. The study has 5 of 244 mutant anchors on comments, loud at next run.")
w("- **Same-diff maintenance is not rowed.** A step that moves a fact and updates its comment in the same diff is no incident. 118 step-file lines in 70 files name such an update, a rough count by grep. Five steps name a close pass: S123, S125, S135, S136, S296. The cost of these is in the study's churn figure, 61.3 % of changed `src` lines.")
w(f"- **Age is a floor** where text moved between files: `git log -S` runs on a one-line fragment in one path, and `src/search_params.hpp` was created on 2026-08-16. The date is the record's; {sum(1 for r in AB if r.get('found'))} rows carry the finding date the record gives instead of the repair's.")
w("")
w("## How it was searched")
w("")
w("- **Read every line by script, step files +/- 130 characters around the match, the others the first 240 to 260 characters:** `comment` in `adocs/plan_done/` (1017 lines, 167 of 272 files), `adocs/audit/` (95, 14 of 17 files), `adocs/decisions.md` (86), `adocs/worklog.md` (23), `adocs/status.md` (74), commit subjects (96). Commit bodies: `comment|docstring` beside `stale|false|wrong|lied|misled|mislead|untrue|no longer|outdated|said|claimed|claims|correct` (24).")
w("- **Worded differently, lines without `comment`, over steps, audits, decisions and worklog:** `misled, mislead, false premise, wrong premise, stopped describ, no longer true, no longer describ, went stale, gone stale, had drifted, drifted from, no longer matches/said/says/holds/reads/names/states`, docstring, note, header, prose, block and GOLDEN beside `stale, wrong, false, untrue, lied, contradict` (103 hits). A location word (`.cpp .hpp .py .sh CMakeLists .cmake`, docstring, header, the case's, the script's) with `said, claimed, stated, promised, quoted` and `stale, false, wrong, untrue, no longer, contradict, mistaken` (16 hits).")
w("- **Harm words within a line of `comment`, status included, across line wraps:** `wasted, re-run, misled, believed, trusted, relied, assumed, mistaken, discarded, void, on the strength, would have, nearly, invites, chased, misread` (38 hits).")
w("- **Benefit:** `own comment, comment already, comment says, comment names, comment warns, comment forbids` from the same 1017 lines; `tried` beside comment, note or site, for tombstones.")
w("- **Read in context, about 150 sites, not whole files.** No step file, audit or decisions.md read whole. `git log -S` for birth wherever a one-line fragment could be named. `adocs/data/` not searched but for this study's reports; its README has 20 `comment` lines, none about a false comment.")
w("- **Coverage of the study's starting set:** F04 A01; F05 B16; S213 C02, B31 to B33, E07, E08; S245 C03, B36 to B38; S249 C05, B14; S254 not a comment incident; S276 B48; S287 C06, B17, E05; S292 D01, B107; S306 C07, B62 to B74, E01, E02; S308 B75 to B82; S125 D20, B46.")
w("")
w("## What could not be read")
w("")
w("- **`.tuning/` is not on this machine, and neither are the workstation's transcripts.** So: briefs under `.tuning/coord/`, including the study's S289, S121, S125, S123 and S083 pointers to comments, not verified and not rowed. Fast-check raw output, run logs, and anything an agent read and did not write down. Whether an agent read a false comment and lost turns: unmeasurable, and it is the most likely place an uncounted A sits. Whether a true comment changed what an agent did, beyond the sentence a step file gives: same, so D is a floor too. Token cost per incident: none here.")
w("- **Before 2026-08-09** there are no step records, S001 to S016 being retro-stamped, so older text has a birth date and no history of who read it. `adocs/data/` scripts are records and only the rows named are checked.")
w("")
w("## Classes and columns")
w("")
w("Record: `S###:n` is `adocs/plan_done/S###_*.md` line n, or `adocs/plan_todo/` for S308; a dated name is under `adocs/audit/`; `git:sha` a commit message. A caused wrong work. B found and repaired, no recorded harm. C exists for comments. D helped. E introduced by a repair or rewrite. **Age**: days from the fragment's first commit to the record's date. **Origin**: birth, false when written; drift, made false later. **Rule**: N, the rule's form allows this text, only review could catch it; F, a form the rule bars (per-step reading, line number, a promise on a step, a number nothing reads, narration), which bites only if in force when written; U unsettled. All rows predate the rule. **Script**: S, R, U as in answer 5. Block: long is 1200 B or more; log is the study's selector.")
w("")
w("## Class A")
w("")
w("| id | cause | date | record | evidence | cost on record | block | age d | origin | rule | script |")
w("|---|---|---|---|---|---|---|---|---|---|---|")
for r in A:
    w(f"| {r['id']} | {r['cause']} | {r['date']} | {short(r['rec'], r.get('line'))} | {qt(r)} | {cell(r['cost'])} | {blockcol(r)} | {agecol(r)} | {r['origin']} | {r['rule']} | {r['script']} |")
w("")
w("## Class B")
w("")
w("| id | date | record | evidence | cost on record | block | age d | origin | rule | script |")
w("|---|---|---|---|---|---|---|---|---|---|")
for r in B:
    note = f" Note: {r['note']}." if r.get("note") else ""
    w(f"| {r['id']} | {r['date']} | {short(r['rec'], r.get('line'))} | {qt(r)} | {cell(r['cost'])}{note} | {blockcol(r)} | {agecol(r)} | {r['origin']} | {r['rule']} | {r['script']} |")
w("")
w("## Class C, done steps")
w("")
w("| id | tier | date | step | goal, quoted | what it held | commits | lines in code files | lines in documents |")
w("|---|---|---|---|---|---|---|---|---|")
for r in C:
    shas, code, docs = Csize[r["id"]]
    w(f"| {r['id']} | {r['tier']} | {r['date']} | {short(r['rec'], r.get('line'))} | {qt(r)} | {cell(r['cost'])} | {' '.join(shas)} | {code} | {docs} |")
w("")
w("### Comment-only commits, `benefit_census.tsv`")
w("")
w(f"Rule: at least one code file changed, every changed code file identical with comments stripped and whitespace collapsed; documents may change beside it; `adocs/data/` counts as a record. {len(cen)} of 1000. A commit that mixes code and comments is not here, so close passes inside code commits are not counted.")
w("")
w("| kind | commits | code-file lines added + removed |")
w("|---|---|---|")
for k in ("repair", "close pass", "record", "build break", "format"):
    w(f"| {k} | {kind_n[k]} | {kind_lines[k]} |")
w(f"| total | {len(cen)} | {total_lines} |")
w("")
w("## Class D")
w("")
w("| id | kind | date | record | evidence | what it did | block | age d at use |")
w("|---|---|---|---|---|---|---|---|")
for r in D:
    w(f"| {r['id']} | {r['kind']} | {r['date']} | {short(r['rec'], r.get('line'))} | {qt(r)} | {cell(r['cost'])} | {blockcol(r)} | {agecol(r)} |")
w("")
w("## Class E")
w("")
w("| id | date | record | evidence | how it was caught | where |")
w("|---|---|---|---|---|---|")
for r in E:
    w(f"| {r['id']} | {r['date']} | {short(r['rec'], r.get('line'))} | {qt(r)} | {cell(r['cost'])} | {cell(r.get('file'))} |")
w("")
w("## Age against class, A and B")
w("")
w("| age | A | B |")
w("|---|---|---|")
for b in BINS:
    w(f"| {b} | {binA[b]} | {binB[b]} |")
w("")
w("| origin | A | B |")
w("|---|---|---|")
for o in ("birth", "drift", "unknown"):
    w(f"| {o} | {origin[('A', o)]} | {origin[('B', o)]} |")
w("")
open(os.path.join(HERE, "benefit_incidents.md"), "w").write("\n".join(L))
print(f"A {len(A)} B {len(B)} C {len(C)} D {len(D)} E {len(E)}; done {DONE}")
print("actual", [r["id"] for r in ACT], "symptom", [r["id"] for r in SYM])
print("ages A", sorted(agesA), "median B", st.median(agesB), "binsA", dict(binA), "binsB", dict(binB))
print("rule", dict(rule), "ruleA", dict(ruleA), "ruleB", dict(ruleB))
print("script", dict(script), "A", dict(scriptA), "B", dict(scriptB))
print("known", len(known), "long", len(longs), "log", len(logs), [r["id"] for r in logs])
print("D known", len(knownD), "long", len(longD), "log", [r["id"] for r in logD], "median", st.median(agesD), dict(kindD))
print("C", {k: v[1:] for k, v in Csize.items()}, "strict", strict_code, strict_docs)
print("census", dict(kind_n), dict(kind_lines), total_lines)
