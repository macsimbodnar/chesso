id:         S163
goal:       a hard-limit timer armed for a previous search can never stop the search that follows it
accepts:    the timer's session check and its stop store become one atomic decision against the current session — a token the timer must CAS against, or a session-tagged stop request the search compares to its own id — so no interleaving lets a stale timer raise the stop flag for a fresh search; the race is first reproduced red with a stress harness (a tight `go movetime 1` / `position` loop pinned with `taskset -c 0`, per the finding), or, if it cannot be made to fire in a stated number of minutes, that is recorded in the stamp and the fix rests on the interleaving argument with the harness kept as the regression net; fast suite green; bench INV-6-identical, since the timer path is outside fixed-depth benching
touches:    src/chesso.cpp, tools/
excludes:   time-management policy — budgets, scaling, soft-limit behaviour; S132 owns the soft limit
decisions:
closes:     2026-08-22_adversarial-F05
blocks:
paused_by:
done:

## Evidence

2026-08-22_adversarial-F05. `stop_search_after_ms` (`src/chesso.cpp:413-424`)
checks `session_id == session` and then stores `stop_search_signal = true` as
two separate operations, against `command_go`'s `stop_and_join_search();
session_id++; stop_search_signal = false;` (`src/chesso.cpp:1326-1329`). A
timer thread preempted between its load and its store sets the flag after the
reset, and the new search dies at its first poll — depth 1, an inexplicable
instant reply (iteration 1 runs on the local `never_stop`, so a move is
always produced). Stale timers are routine, not exceptional: a move normally
ends at the soft limit, so its hard timer is still sleeping when the next
`go` arrives, every move of every game. Under SPRT the noise hits both sides
equally; in rated play it is a real, if rare, strength leak.

## Cost

Small synchronization change, a stress harness, no match.
