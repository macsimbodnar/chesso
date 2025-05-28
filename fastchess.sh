#!/bin/bash

# fastchess -engine cmd=/home/max/ws/chesso/build/src/chesso name=chesso_experimental -engine cmd=/usr/games/chesso_v0.2.1 name=chesso_v0.2.1 -each tc=8+0.08 -rounds 15000 -repeat -concurrency 16 -recover -openings file=/home/max/ws/chesso/.no_git/8moves_v3.pgn format=pgn -sprt elo0=0 elo1=5 alpha=0.05 beta=0.05
# fastchess.exe -engine cmd=Engine1.exe name=Engine1 -engine cmd=Engine2.exe name=Engine2 -each tc=10+0.1 -rounds 200 -repeat -concurrency 4

# fastchess --compliance /usr/games/chesso 


fastchess \
  -engine cmd=/usr/games/chesso_bitboard name=chesso_bitboard \
  -engine cmd=./build/src/chesso name=chesso_candidate \
  -openings file=/home/max/ws/chesso/.no_git/8moves_v3.pgn format=pgn \
  -each tc=10+0.2 \
  -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 \
  -rounds 200 \
  -repeat \
  -concurrency 12 \
  -log engine=true file=/tmp/fastchess.log


# fastchess -quick cmd=/usr/games/chesso_v0.3.0 cmd=/usr/games/chesso_v0.2.1 book=/home/max/ws/chesso/.no_git/8moves_v3.pgn