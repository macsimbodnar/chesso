id:         S173
goal:       `make_book build` replaces a book atomically, so a failed write leaves the previous book where it was instead of destroying it
accepts:    `build` writes to a temporary file beside the destination and renames it into place only after the stream is verified good, so a short write, a full volume or a kill mid-write leaves any pre-existing file at `--out` byte-for-byte unchanged; the temporary is removed on every failure path and nothing is left beside the destination; `rename` within the same directory is used rather than a copy, so the replacement is atomic on the filesystems this project runs on; reproduced red before the change on a 1 MB HFS ram disk with a pre-existing file at `--out` -- the file is destroyed today -- and green after; the shipped book's digest is unchanged by the change, `77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58`; `DEV_MANUAL.md`'s "loadable does not mean complete" paragraph is amended to say what the tool now guarantees
touches:    tools/make_book.cpp, tests/test_make_book_tools.sh, DEV_MANUAL.md
excludes:   the engine's own book loading, which reads and never writes; `dump`, which opens read-only; making any other tool in `tools/` write atomically, which is the same idea and a separate decision
decisions:  DEC-131
closes:
blocks:
paused_by:
author:     Claude Opus 5 (coordinator)
done:       2026-09-07. `build` writes to `<out>.tmp` beside the destination and renames it over `--out` only after the write, `fsync` and `close` all succeed; every failure the process lives through unlinks the temporary and exits 1 with `book not written` and `strerror`. `SIGXFSZ` is ignored in `main`, without which the limit kills the tool instead of being reported. **Red first, on the workstation**: the three new fixture properties (9, 10, 11 -- 7 and 8 are S178's) gave two `FAIL:` lines on today's binary, `atomic: the destination changed: 0 bytes` and `atomic: no refusal message:`, exactly what the guide predicted; by hand under `ulimit -f 0` the pre-change tool exits **153** (SIGXFSZ, 128+25), having already truncated a 13-byte pre-existing book to 0. **The partial-write form reproduced here too**, which S146 had only on macOS: pre-change, `ulimit -f 200` over `books/8moves_v3.pgn` onto a pre-existing book left **204800 bytes** at `--out` and `dump` read it happily (`heaviest 1: 0844931a6ef4b9a0 g1f3 weight 2527`) -- a truncated book that passes every validator in the tree. **Green after**: fixture `ok`, exit 0; by hand, exit 1, `wrote 204800 of 2755712 bytes to '<out>.tmp': File too large -- book not written`, the destination still `previous book` at 13 bytes, no temporary left. **The success path is unchanged, measured not argued**: the shipped book rebuilt through the new write path is 2755712 bytes, sha256 `77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58`, `cmp`-identical to `src/openings.bin`, 1.15 s. **No `src/` file is in the diff**, so the engine binary is byte-identical, no `Bench:` line is owed (and DEC-140 binds from S189's completing commit anyway, S189 being open) and INV-6 is not owed -- `make_book` is its own executable. Gate **27/27 in both builds**, `clang-format` clean under `CLANG_FORMAT_MAJOR=22` (DEC-146); `plan_prose_check.py` exit 0, 0 touches flagged. **The three deferred questions are answered**: the owner chose the **fixed** `<out>.tmp` over `mkstemp`, so the `accepts` holds as written and one kill leaves one reclaimable file (**DEC-149**); the mode question answers itself under that choice, since `O_WRONLY|O_CREAT|O_TRUNC` at 0666 is exactly what the replaced `std::ofstream` asked for and `rename` carries it onto the destination; `tests/test_make_book_tools.sh` joined `touches:`, the shipped-digest check staying by hand and out of the ctest. `DEV_MANUAL.md`'s "`loadable` does not mean complete" paragraph now states the guarantee and carries the `ulimit -f` reproduction as the root-free form of S146's ram disk. `MANUAL.md` checked -- no UCI surface moves, `test_uci_surface` not refreshed. `adocs/specs.md` checked -- it describes the shipped file, the reproducible build command and the S174/S201 parser gate, none of which move; no change.

## Why this exists

Found by S146's fast check over its own diff, 2026-09-03, and it is the *fix*
the reviewer proposed rather than the problem it reported.

**The reported problem was not real and that is worth stating first**, so nobody
re-derives it. The finding was that S146's new failure path calls
`std::remove(options.out.c_str())` and so "deletes any file at the output path,
including pre-existing files not created by this run". It does delete one. It
destroys nothing, because `std::ofstream output(options.out, std::ios::binary)`
(`tools/make_book.cpp:422`) opens `"wb"` and truncates the file the instant it
succeeds -- verified directly: a bare open with no write and no `remove` took a
nine-byte file to zero bytes. By the time the guard runs, the previous contents
are already gone. Removing the remnant is strictly better than leaving it,
because of the invariant below.

**What is real is that the whole operation is not atomic.** The tool destroys
the old book before it knows it can write the new one, and that is the thing
worth fixing:

- entries are sixteen bytes and sorted by key, so **any prefix of a valid book
  is a valid book**. `make_book dump` and the engine's loader both call a
  truncated file `loadable`; neither check can notice truncation. S146 measured
  it -- 901120 of 2755712 bytes written, 56320 entries, verdict `loadable`,
  engine loaded it.
- so the failure mode is not "the write failed and I noticed". It is "the write
  failed, the old book is gone, and what is on disk passes every validator in
  the tree". S146's guard turns that into a clean error and an absent file,
  which is safe but still loses the old book.

Write beside the destination, `rename` on success. The old book survives every
failure, and the replacement is never observed half-done by anything reading
concurrently.

## Cost

Small: the guard S146 added is already the place the temporary is cleaned up
from, and the shipped book's digest must not move, which is the check that the
change is behaviour-preserving on the success path.

## Not urgent

`--out` is pointed at `src/openings.bin` about once, and that file is in git.
The class of bug is the reason to fix it, not the exposure.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

`tools/make_book.cpp` is the only program in the tree that produces the book the
engine ships with, `src/openings.bin`. Its `build` function collects the entries,
then opens the destination with `std::ofstream output(options.out,
std::ios::binary)`, which truncates whatever is at `--out` the instant it opens;
only afterwards does it learn whether the write worked. A write that fails -- a
full volume, a file-size limit, a kill -- has therefore already destroyed the
previous book. S146's guard after the write deletes the partial file and exits 1,
which stops a truncated-but-`loadable` book from shipping, but the previous book
is still gone. This step writes to a temporary beside the destination and
`rename`s it into place only once the write is verified. Nothing in the engine
changes; a red-first test and the shipped book's digest decide it, not a match.

### 2. The technique as published

Write-to-temporary-then-rename is the standard way to replace a file so a reader
sees the old file or the new one and never anything between. POSIX `rename()`
(https://pubs.opengroup.org/onlinepubs/9699919799/functions/rename.html): "a link
named new shall remain visible to other threads throughout the renaming operation
and refer either to the file referred to by new or old before the operation
began", and "If the rename() function fails for any reason other than [EIO], any
file named by new shall be unaffected"; across file systems it fails `EXDEV`,
which is why the temporary lives in the destination's own directory and never
under `$TMPDIR`. Linux `rename(2)` (https://man7.org/linux/man-pages/man2/rename.2.html):
"If newpath already exists, it will be atomically replaced, so that there is no
point at which another process attempting to access newpath will find it
missing." POSIX `mkstemp()` (https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkstemp.html)
creates the temporary "as if by a call to: open(pathname, O_RDWR|O_CREAT|O_EXCL,
S_IRUSR|S_IWUSR)" from a template ending in six `X`; `O_EXCL` removes the race
between choosing a name and creating it.

Durability: Moyer, "Ensuring data reaches disk", LWN 2011
(https://lwn.net/Articles/457667/) gives the five-step form -- temporary on the
same file system, write, `fsync()` it, `rename()`, `fsync()` the directory -- and
the ext4 admin guide (https://www.kernel.org/doc/html/latest/admin-guide/ext4.html)
names `open("foo.new")/write/close/rename("foo.new","foo")` without `fsync` as
the source of a "zero-length" file after a crash under delayed allocation. It
does not matter for correctness here: the book goes into git and is verified by
digest, so a power cut between rename and flush costs a rebuild. `fsync(fd)` on
the temporary is kept anyway -- one line, unmeasurable at 2.7 MB, and it closes
the zero-length case only a `cmp` would otherwise catch. The directory `fsync` is
dropped: a rename lost to a crash leaves the old book in place, which is the state
this step guarantees.

What makes the test portable is POSIX `setrlimit`
(https://pubs.opengroup.org/onlinepubs/9699919799/functions/getrlimit.html) on
`RLIMIT_FSIZE`: "If a write or truncate operation would cause this limit to be
exceeded, SIGXFSZ shall be generated for the thread. If the thread is blocking,
or the process is catching or ignoring SIGXFSZ, continued attempts to increase
the size of a file from end-of-file to beyond the limit shall fail with errno set
to [EFBIG]"; and "Soft limits may be changed by a process to any value that is
less than or equal to the hard limit", so an unprivileged shell forces a failed
write with `ulimit -f`. The tool must ignore `SIGXFSZ` for the failure to arrive
as an error rather than a kill (section 5).

Chesso's form: `mkstemp` on `<out>.tmp.XXXXXX`, a `write()` loop, `fsync`,
`fchmod`, `close`, `rename`; `unlink` the temporary on every failure the process
lives to see; `SIGXFSZ` ignored in `main`. A prototype of exactly this ran on the
macOS machine on 2026-09-05: under `ulimit -f 200` it printed `write: File too
large after 204800 bytes`, exit 1, destination byte-identical, no temporary; on a
full 1 MB HFS ram disk, `No space left on device after 0 bytes` with the same
outcome; with no limit, 2755712 bytes and exit 0. An `std::ofstream` opened on
the `mkstemp` name also reports the failure through `good()` once `SIGXFSZ` is
ignored (observed), so the stream is viable; the descriptor path is recommended
because it yields `errno` for the message and a descriptor for `fsync`.

### 3. What chesso has today, and where the change plugs in

All in `tools/make_book.cpp`, inside `build(const build_options_t& options)`.
Untouched: the accumulation into `std::vector<uint8_t> bytes` through `write_be`;
the S174 gate `rejected_games > 0 && !options.allow_cut_short`, which returns
"before the output is touched" (its comment) and so keeps the existing test's
property 3 true with no temporary ever created; and the report `printf`s ending
`bytes %llu -> %s`, which follow a successful rename. **Replaced:** the block from
`std::ofstream output(options.out, std::ios::binary)` and its `output.fail()`
check (`cannot write '%s'`) through `output.write(...)`, `output.close()` and the
S146 guard `if (!output.good())` that prints `short write to '%s' -- book not
written` and calls `std::remove(options.out.c_str())`. (The line-numbered
citation in "Why this exists" above, 422 when written, points at this
`std::ofstream output(...)` statement and has drifted; the symbol is the durable
reference, DEC-135.)

The replacement, in order:

1. `options.out + ".tmp.XXXXXX"` copied into a `std::vector<char>` with a
   trailing NUL -- `mkstemp` rewrites the buffer in place. `int fd =
   mkstemp(buffer.data())`; on -1 print `cannot write '%s': %s` with
   `strerror(errno)` and return 1; `--out` is untouched and nothing needs
   cleaning.
2. A write loop: `ssize_t n = write(fd, bytes.data() + done, bytes.size() -
   done)`; retry on `n < 0 && errno == EINTR`; any other `n < 0` is the failure
   path. `write` may return fewer bytes than asked with no error, hence a loop.
3. `fsync(fd)`; then the mode: `mkstemp` creates 0600 where the old `ofstream`
   gave `0666 & ~umask` (0644 on both machines) -- git tracks only the
   executable bit, but a rebuilt `src/openings.bin` should not silently become
   owner-only, so `mode_t mask = umask(0); umask(mask); fchmod(fd, 0666 &
   ~mask)`; then `close(fd)`, checked -- some file systems first report a write
   error there.
4. `std::rename(buffer.data(), options.out.c_str())` -- `<cstdio>` is included
   and it is `rename(2)`; on failure print the reason and fall into cleanup.
5. One cleanup for every failure after `mkstemp` succeeded: `close(fd)` if still
   open, `unlink(buffer.data())`, return 1. Keep S146's phrase `book not
   written` so the manual and the test grep one stable string, and append
   `strerror(errno)` so `File too large` and `No space left on device` are told
   apart.
6. In `main`, first statement: `std::signal(SIGXFSZ, SIG_IGN)`. New includes
   `<cerrno>`, `<csignal>`, `<fcntl.h>`, `<sys/stat.h>`, `<unistd.h>`. No file
   under `tools/` or `src/` includes a POSIX header today; this is the first,
   deliberately -- `std::filesystem::rename` exists but nothing standard gives
   `mkstemp` or `fsync`. `.clang-format` is `BasedOnStyle: Chromium` and sorts
   the block.

`dump` reads through `std::ifstream` and is excluded, as is the engine's loader.
No `src/` file changes.

### 4. Constants and seeds

None in the DEC-105 sense; `src/search_params.hpp` is not opened. The two
literals introduced are the suffix `.tmp.XXXXXX` (six `X`, POSIX's minimum) and
`0666 & ~umask`, which reproduces what the replaced `ofstream` gave.

### 5. Interactions and traps

No search hazard applies -- no pruning, ordering band, accumulator, table bound,
improving flag or clock is touched. The traps are in the file system, each met
while preparing this guide on 2026-09-05:

- **`SIGXFSZ` kills by default, and today's tool dies of it.** `ulimit -f 200`
  then `make_book build books/8moves_v3.pgn --out book.bin` over a pre-existing
  9-byte book: `Filesize limit exceeded: 25`, exit 153, and a **204800-byte file
  left at `--out`** -- truncated, sorted, `loadable`. S146's guard never ran;
  the process was dead. Strictly worse than the ENOSPC case S146 reproduced,
  which left no file, and the reason `main` ignores the signal: the limit then
  arrives as `EFBIG` from `write`, which the loop reports and cleans up after.
- **A failed write is not always partial.** On the full ram disk the first
  `write` of 2755712 bytes failed at 0 bytes. Assert what the accepts says --
  destination unchanged, no temporary -- never "some bytes got written".
- **`/dev/full` does not fit.** Linux only (https://man7.org/linux/man-pages/man4/full.4.html,
  "Writes to the /dev/full device fail with an ENOSPC error"), and the temporary
  goes beside the destination, where `/dev/full.tmp.XXXXXX` is not creatable by
  a user. `ulimit -f` is the root-free reproduction on both operating systems.
- **`ulimit -f` counts 1024-byte units in bash, 512 in POSIX mode**
  (https://www.gnu.org/software/bash/manual/html_node/Bash-Builtins.html);
  observed, `ulimit -f 200` stopped the file at 204800 bytes. The fixture book is
  64 bytes, under one unit, so the ctest uses `ulimit -f 0`, which fails the
  first byte; the partial-write form is the shipped PGN under `ulimit -f 200`.
- **The limit covers every regular file the child writes, stderr included.**
  Take the tool's output through a pipe -- `out=$( ( ulimit -f 0; "$make_book"
  ... ) 2>&1 )` -- not a file, or the error message is what exceeds the limit.
- **`kill -9` mid-write leaves the temporary.** The destination is untouched,
  which is the accepts' first clause, but `<out>.tmp.XXXXXX` remains and no test
  can assert otherwise; section 10, question 1.
- **`hdiutil attach` prints the device followed by tabs**: use `awk '{print
  $1}'`; `tr -d ' '` keeps them and `diskutil` answers `Unable to find disk`.
  `diskutil eraseVolume HFS+ <name> $DEV` mounts at `/Volumes/<name>` with no
  `sudo`; S146's `newfs_hfs` plus `mount -t hfs` needs it.
- **macOS `/bin/bash` is 3.2** (`.moltke.local.md`, S167): keep the script's
  `set -uo pipefail` without `errexit`, use `$( ( ... ) )` for the limited
  subshell, and run the script once on that machine before closing.
- **A Linux ram disk without root is not guaranteed.** `unshare -Urm sh -c
  'mount -t tmpfs -o size=1m none /mnt/x && ...'`
  (https://man7.org/linux/man-pages/man1/unshare.1.html: `-U` user namespace,
  `-r` map to root, `-m` mount namespace) works where unprivileged user
  namespaces are enabled; whether the workstation allows it is unverified. An
  optional second by-hand check, never the test.

### 6. Tests

Red first, in `tests/test_make_book_tools.sh`, already wired by
`tests/CMakeLists.txt`'s `add_test(NAME test_make_book_tools ...)` under `fast`.
Extend the header's numbered list and add three properties after 6, reusing
`control.pgn`, `control.bin` and `illegal.pgn`:

```bash
# 7. A failed write leaves a pre-existing destination byte-identical and no
#    temporary beside it. ulimit -f 0 fails the first write with EFBIG (the tool
#    ignores SIGXFSZ); RLIMIT_FSIZE is per process, so the subshell scopes it.
#    Output goes through a pipe: a file would hit the same limit.
printf 'previous book' > "$tmp/keep.bin"
cp "$tmp/keep.bin" "$tmp/keep.before"
out=$( ( ulimit -f 0; "$make_book" build "$tmp/control.pgn" --out "$tmp/keep.bin" ) 2>&1 )
rc=$?
[[ $rc -ne 0 ]] || fail "atomic: build exited 0 under a zero file-size limit"
cmp -s "$tmp/keep.bin" "$tmp/keep.before" || fail "atomic: destination changed"
[[ -z "$(ls "$tmp" | grep '\.tmp\.')" ]] || fail "atomic: temporary left: $(ls "$tmp")"
grep -q 'book not written' <<< "$out" || fail "atomic: no refusal message: $out"

# 8. Success replaces a pre-existing destination and leaves no temporary.
printf 'previous book' > "$tmp/replace.bin"
"$make_book" build "$tmp/control.pgn" --out "$tmp/replace.bin" > "$tmp/replace.out" 2>&1 \
  || fail "replace: build exited non-zero"
cmp -s "$tmp/replace.bin" "$tmp/control.bin" || fail "replace: not the control book"
[[ -z "$(ls "$tmp" | grep '\.tmp\.')" ]] || fail "replace: temporary left"

# 9. The cut-short refusal still precedes any write: a pre-existing file survives.
printf 'previous book' > "$tmp/refused.bin"
"$make_book" build "$tmp/illegal.pgn" --out "$tmp/refused.bin" > /dev/null 2>&1
[[ "$(cat "$tmp/refused.bin")" == "previous book" ]] || fail "refused: destination changed"
```

Observed red on today's binary (macOS, 2026-09-05): property 7 exits 153, so the
exit check passes; the destination is **0 bytes**, so `cmp` fails; no temporary,
so that passes; the message is absent, so the last check fails -- two failures,
the red to record. Properties 8 and 9 pass today and guard the new code (8 the
rename, 9 the gate staying ahead of `mkstemp`). One test alone:
`ctest --test-dir build -R test_make_book_tools --output-on-failure`.

The accepts' literal reproduction, by hand on the macOS machine, no `sudo`:

```bash
DEV=$(hdiutil attach -nomount ram://2048 | awk '{print $1}')   # 1 MB
diskutil eraseVolume HFS+ s173tiny "$DEV"                       # mounts /Volumes/s173tiny
printf 'previous book' > /Volumes/s173tiny/book.bin
build/tools/make_book build books/8moves_v3.pgn --out /Volumes/s173tiny/book.bin; echo "exit $?"
ls -la /Volumes/s173tiny | grep book
hdiutil detach "$DEV"
```

Before the change, observed: `short write to ... -- book not written`, exit 1,
**no file** -- the previous book is gone. After: exit 1 and `previous book`
still there, 13 bytes. On the Linux workstation the by-hand form is `ulimit -f
200` in a subshell against a pre-existing file -- the same class, another
`errno`.

No mutant, no precondition guard, no Debug self-play: DEC-141's second tier is
for a pruning, reduction or extension rule, or a step touching `make_move`,
`unmake_move`, the generator or the search, and this touches none. No golden
enters `tests/`; the one this step leans on is the shipped digest, already named
at its sites in `DEV_MANUAL.md` ("The engine's own opening book") and
`adocs/specs.md` and re-derived by the `make_book build` command beside it.
INV-6 (`adocs/specs.md`: "no test: a procedure. `tools/search_bench.py` node
counts and best moves for the neutral half") is not owed: `make_book` is its own
executable (`tools/CMakeLists.txt`, `add_executable(make_book make_book.cpp)`),
no `src/` file changes, and the engine binary is byte-identical before and after.
Section 7 is the check that stands in for it.

### 7. Measurement

No lane. No SPRT -- `OwnBook` defaults false, no measurement on record has played
a book move (S146, S158), and the book's bytes do not change; no `hyperfine`, the
engine is not rebuilt. The measurement is the success path, run before the change
(passes today, observed `real 0.97` s and byte-identical) and after, both digests
in the stamp:

```bash
build/tools/make_book build books/8moves_v3.pgn --out /tmp/S173_check.bin
shasum -a 256 /tmp/S173_check.bin   # 77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58
cmp /tmp/S173_check.bin src/openings.bin && echo identical
```

`src/openings.bin` is not rewritten by this step; if it is rebuilt in place to
prove the mode, `git status` must still show it unchanged. No `adocs/data/S173_*`
script is owed -- there is no run to pre-register.

### 8. Completion checklist

1. Test extended and observed red (two failures on property 7); the code; green.
2. The gate, both builds: `cmake --build build -j8 && ctest --test-dir build -L
   fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
   build-tune -L fast --output-on-failure && ./clang-format.sh --check`. Apple
   clang builds `-Werror`: an unused variable fails there and not on Linux.
3. Section 7's digest and `cmp` before and after; the by-hand ram-disk (macOS) or
   `ulimit -f 200` (Linux) red and green with their outputs.
4. Bench line: not owed, no `src/` file is touched. If `tools/gate.sh` (S189)
   exists by then, run it and do what it says.
5. `DEV_MANUAL.md`, "The engine's own opening book", the paragraph opening
   "**`loadable` does not mean complete**": its sentence "`build` now checks the
   stream after the write, deletes the partial file and exits non-zero" becomes
   the guarantee -- `build` writes beside the destination and renames over it
   only after the write is verified, so a failed write (full volume, file-size
   limit, kill) leaves any previous book at `--out` byte-for-byte unchanged and
   no temporary behind. Keep the closing clause about a book reaching you by
   another route -- the digest is still the only completeness check for a file
   the tool did not write -- and add the `ulimit -f` reproduction as the
   portable form of S146's ram disk.
6. `MANUAL.md`: check, expect no change; no UCI surface moves, `test_uci_surface`
   is not refreshed. `adocs/specs.md`: the book paragraph describes the shipped
   file and the S174 gate, not the write path; check, expect no change. Record
   both conclusions -- the DOCS rule makes the checking the requirement.
7. Stamp: the red (exit 153, 0-byte destination, message absent), the green, the
   by-hand reproduction before and after, both digests, the gate counts for both
   builds, `clang-format` clean, the two docs conclusions, and the `SIGXFSZ`
   decision in one sentence.
8. `plan.md` and `status.md` are the coordinator's; hand it the stamp text.
   Commit subject imperative, body citing S173 and why, no Bench line.

### 9. Sources read

- https://pubs.opengroup.org/onlinepubs/9699919799/functions/rename.html -- atomic visibility of `new`, `EXDEV`, `new` unaffected on failure. Fetched.
- https://man7.org/linux/man-pages/man2/rename.2.html -- "atomically replaced ... no point at which another process ... will find it missing". Fetched.
- https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkstemp.html -- `O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR`, six trailing `X`. Fetched.
- https://pubs.opengroup.org/onlinepubs/9699919799/functions/getrlimit.html -- `RLIMIT_FSIZE`, `SIGXFSZ`, `EFBIG` when ignored, soft limits lowerable. Fetched.
- https://lwn.net/Articles/457667/ -- Moyer, "Ensuring data reaches disk", the five-step form. Fetched.
- https://www.kernel.org/doc/html/latest/admin-guide/ext4.html -- `auto_da_alloc`, the replace-via-rename zero-length hazard. Fetched.
- https://man7.org/linux/man-pages/man4/full.4.html -- `/dev/full` writes fail `ENOSPC`. Fetched.
- https://www.gnu.org/software/bash/manual/html_node/Bash-Builtins.html -- `ulimit -f` in 1024-byte units, 512 in POSIX mode. Fetched.
- https://man7.org/linux/man-pages/man1/unshare.1.html -- `-U`, `-m`, `-r`; no tmpfs example. Fetched; the workstation's unprivileged-namespace support is unverified.
- Repository: `tools/make_book.cpp` (`build`, `main`, `build_options_t`, `write_be`, `dump`), `tests/test_make_book_tools.sh`, `tests/CMakeLists.txt`, `tools/CMakeLists.txt`, `DEV_MANUAL.md` "The engine's own opening book", `adocs/specs.md` (INV-6 row, book paragraph), `adocs/plan_done/S146_openings_book_provenance.md` (the ram-disk reproduction), `adocs/plan_done/S174_san_parser_fails_closed.md` (stamp template), DEC-131, DEC-135, `.moltke.local.md`, `.clang-format`.
- Experiments 2026-09-05, macOS machine, scratchpad only: today's binary under `ulimit -f 200`, `ulimit -f 0` and on a 1 MB HFS ram disk; a 60-line `mkstemp`/`write`/`fsync`/`rename` prototype with `SIGXFSZ` ignored under both; the full build timed at 0.97 s and `cmp`-identical to `src/openings.bin`.

### 10. Questions deferred to the owner

1. **"the temporary is removed on every failure path and nothing is left beside
   the destination" cannot hold for `kill -9` mid-write** -- the process is dead
   before it can `unlink`. Proposed reading: every failure path the process lives
   through; a `SIGKILL` leaves `<out>.tmp.XXXXXX`, identifiable by name, and the
   destination untouched. The stronger property needs a fixed name `<out>.tmp`,
   overwritten by the next run so at most one is ever left, at the cost of two
   concurrent builds clobbering each other's temporary.
2. **`touches:` omits `tests/test_make_book_tools.sh`**, where the red-first
   test lives (S141 is the precedent: touches names the landing file). If the
   shipped digest check is wanted inside the ctest rather than by hand -- the
   build takes about 1 s -- `tests/CMakeLists.txt` joins `touches:` too, to pass
   `${CMAKE_SOURCE_DIR}` as a third argument.
3. File mode after replacement: `0666 & ~umask`, what the replaced `ofstream`
   gave, is picked as a low-stakes detail. Say if the destination's existing mode
   should be preserved instead.
