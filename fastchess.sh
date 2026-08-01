#!/usr/bin/env bash
set -euo pipefail

# NOTE: --fast will run the fast version.

# reference="/usr/games/chesso"
reference="/home/max/ws/chesso/.no_git/chesso_bitboard_minimal_eval"
candidate="/home/max/ws/chesso/build/src/chesso"
book="/home/max/ws/chesso/.no_git/8moves_v3.pgn"
tc="10+0.2"
concurrency=12

# adjudication cuts dead games, gets to a verdict faster
adjudication="-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400"

if [[ "${1:-}" == "--fast" ]]; then
  # few hundred games
  sprt="elo0=0 elo1=10 alpha=0.10 beta=0.10"
  rounds=1500
  logfile="/tmp/fastchess_fast.log"
else
  # SPRT will stop by it self
  sprt="elo0=0 elo1=5 alpha=0.05 beta=0.05"
  rounds=20000
  logfile="/tmp/fastchess_full.log"
fi

fastchess \
  -engine cmd="$candidate" name=candidate \
  -engine cmd="$reference" name=reference \
  -openings file="$book" format=pgn order=random \
  -each tc="$tc" option.Hash=16 option.Threads=1 \
  -sprt $sprt model=normalized \
  $adjudication \
  -rounds "$rounds" \
  -repeat \
  -concurrency "$concurrency" \
  -log engine=true file="$logfile"
