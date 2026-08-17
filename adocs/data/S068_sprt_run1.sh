#!/usr/bin/env bash
# S068: full SPRT, margin 75 candidate against 7f15ac4 (margin 100).
# Prints a terminal marker as its last action so a watcher can exit on it
# rather than holding a bare `tail -f` open forever. DEC-061.
cd /home/max/ws/chesso || { echo "S068_SPRT_FAILED cd"; exit 1; }

start=$(date +%s)
REF=7f15ac4 ./fastchess.sh
status=$?
end=$(date +%s)

echo
echo "elapsed_seconds=$(( end - start ))"
if [[ $status -eq 0 ]]; then
  echo "S068_SPRT_DONE status=0"
else
  echo "S068_SPRT_FAILED status=$status"
fi
