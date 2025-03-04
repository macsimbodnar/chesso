#!/bin/bash

if [ $# -gt 0 -a "$1" != "--check" ]; then
    echo "Usage: $0 [--check]"
    exit 0
fi

if [ ! -f CMakeLists.txt ]; then
    echo "Please run this script from the root folder."
    exit 1
fi

FILES=$(git ls-files | grep -E '\.(c|cc|cpp|h|hpp|hh)$' | grep -E -v '3rd_parties')

TOOL=clang-format-15
if [ ! -x "$(command -v $TOOL)" ]; then
    # clang-format-15 not available, use default
    TOOL=clang-format
    if [ ! -x "$(command -v $TOOL)" ]; then
        echo "clang-format not available."
        echo "Please install clang-format (preferred version: 15)"
        echo "On ubuntu run apt install clang-format-15"
        exit 1
    fi

    VERSION=$($TOOL --version | grep -E -o "version [0-9\.]+")

    if [ "$VERSION" != "version 15.0.7" ]; then
        echo "Warning: using clang-format $VERSION."
        echo "Circle CI uses version 15.0.7."
        echo "This may lead to formatting errors."
    fi
fi

EXTRA_ARGS=""
if [ "$1" == "--check" ]; then
    EXTRA_ARGS="--dry-run -Werror"
fi

$TOOL $EXTRA_ARGS -i $FILES
