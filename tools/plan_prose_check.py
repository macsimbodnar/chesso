#!/usr/bin/env python3
"""Check that plan.md's prose does not describe a completed step as pending.

plan.md is second in the reading order and its prose is what a cold session
reads before the ordered list. The list is maintained by the workflow checker;
the prose around it is not, so it goes stale every time a step completes. That
has now happened three times: 2026-08-13_plan_review-F05, then
2026-08-13_plan_review.2-F07 after S048 cleared it, then again the moment S033
completed. S062 is the third repair and this is the check that makes the fourth
cheap to catch.

Sentence-wise, not line-wise. The prose is hard-wrapped, so an id and the claim
about it routinely sit on different lines; a line-based version of this check
missed two of the five stale claims S062 found.

    tools/plan_prose_check.py                 # checks adocs/plan.md
    tools/plan_prose_check.py path/to/plan.md # or an explicit file

Exits non-zero when any sentence is flagged. It reports, it does not rewrite:
which tense a sentence should take is a judgement, and the fix belongs in the
same commit as the completion that made it stale.
"""
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLAN_DIRS = ("plan_done", "plan_current", "plan_todo")

# Claims that only make sense about work that has not happened yet. Extend this
# when a new phrasing slips through, rather than loosening what it matches.
#
# "lead what is left" is here because it slipped through inside S062 itself: the
# sentence was written while S062 was current and went stale the moment the
# checker moved it to plan_done/, in the same minute. A sentence that states a
# live position in the queue goes stale by construction; the durable fix is to
# write the placement as history and let the list carry the order.
PENDING = re.compile(
    r"is next|are next|still pending|everything pending|"
    r"at the front of the pending|is at the front|front of the pending list|"
    r"pending order|leads the pending|lead what is left|leads what is left|"
    r"will be|is planned|comes next|is the next", re.I)

ENTRY = re.compile(r"^\d+\.\s+S\d{3}")
STEP_ID = re.compile(r"S\d{3}")


def state(sid, adocs):
    """Where a step id lives, or 'retired' when its file is gone."""
    for d in PLAN_DIRS:
        directory = os.path.join(adocs, d)
        if not os.path.isdir(directory):
            continue
        for name in os.listdir(directory):
            if name.startswith(sid + "_"):
                return d
    return "retired"


def prose_paragraphs(text):
    """Everything that is not an entry of the ordered list. plan.md says an id
    named in a sentence is prose and the list is the order; this is that split
    made mechanical."""
    kept = [l for l in text.split("\n") if not ENTRY.match(l)]
    return "\n".join(kept).split("\n\n")


def sentences(paragraph):
    flat = " ".join(paragraph.split())
    # Split on a full stop followed by a capital or a markdown marker, so that
    # "S044 to S052" and "10.3 cp" survive where a naive split on "." does not.
    return re.split(r"(?<=[.!?])\s+(?=[A-Z*`])", flat)


def check(path, adocs):
    text = open(path).read()
    flagged = []
    ids = set()

    for paragraph in prose_paragraphs(text):
        for sentence in sentences(paragraph):
            found = STEP_ID.findall(sentence)
            ids |= set(found)
            done = sorted({s for s in found if state(s, adocs) == "plan_done"})
            if done and PENDING.search(sentence):
                flagged.append((done, sentence))

    completed = sorted(s for s in ids if state(s, adocs) == "plan_done")
    print(f"{path}: {len(ids)} ids in prose, {len(completed)} of them completed")

    for done, sentence in flagged:
        print(f"  FLAG {' '.join(done)}")
        print(f"       {sentence[:160]}")

    print(f"  sentences flagged: {len(flagged)}")
    return len(flagged)


def main():
    adocs = os.path.join(REPO, "adocs")
    paths = sys.argv[1:] or [os.path.join(adocs, "plan.md")]
    return 1 if any(check(p, adocs) for p in paths) else 0


if __name__ == "__main__":
    sys.exit(main())
