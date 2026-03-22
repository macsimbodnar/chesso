#!/bin/bash

zig c++ \
  src/bitboard.cpp  \
  src/chesso.cpp  \
  src/evaluation.cpp  \
  src/openings.cpp  \
  src/search.cpp  \
  src/transposition_table.cpp  \
  src/utils.cpp \
  -o build/chesso \
  -std=gnu++20
  #  -O3 -DNDEBUG