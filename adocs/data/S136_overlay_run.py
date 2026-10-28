#!/usr/bin/env python3
"""S136. A tracked sweep script run against an uncommitted candidate.

    python3 adocs/data/S136_overlay_run.py SCRIPT FILE[,FILE...] -- ARGS...
    python3 adocs/data/S136_overlay_run.py adocs/data/S156_mined_floor_sweep.py \\
        src/evaluation.cpp -- --ref HEAD --floor 159 --jobs 12

`adocs/data/S156_mined_floor_sweep.py` and `S154_floor_margin_sweep.py` check
`--ref` out in a throwaway worktree, so they measure commits. S136's phase 2
candidate is the parent plus a working-tree paste, and the brief that runs it
commits nothing (S135 and S296 made commit objects on no branch instead). This
runs SCRIPT unchanged except that, right after its `git worktree add`, the
named files are copied from the repository the script lives in -- the working
tree -- into the throwaway tree. It is S154's own `overlay` argument, applied
to a script that has none and to a source file. Every patch, build and reading
after that is the script's.

Standard library only. Nothing here asks a chess question.
"""

import importlib.util
import os
import shutil
import sys


def main():
    if len(sys.argv) < 4 or sys.argv[3] != "--":
        raise SystemExit(__doc__)

    script, files = sys.argv[1], sys.argv[2].split(",")
    spec = importlib.util.spec_from_file_location("sweep", script)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    original = module.run

    def run(command, cwd):
        out = original(command, cwd)

        # Both scripts call run(["git", "worktree", "add", "--detach", tree,
        # ref], REPO) before any patch, so the overlay lands under every one.
        if command[:3] == ["git", "worktree", "add"]:
            tree = command[-2]

            for relative in files:
                shutil.copyfile(os.path.join(module.REPO, relative),
                                os.path.join(tree, relative))
                print("  overlaid %s from %s" % (relative, module.REPO))

            sys.stdout.flush()

        return out

    module.run = run
    sys.argv = [script] + sys.argv[4:]
    return module.main()


if __name__ == "__main__":
    sys.exit(main())
