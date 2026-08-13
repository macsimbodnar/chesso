id:         S043
goal:       delete the CMAKE_TOOLCHAIN_FILE line that names a file that does not exist
accepts:    a fresh configure in a clean directory succeeds and selects the same compiler as before; no reference to toolchain.cmake remains
touches:    CMakeLists.txt
excludes:   introducing an actual toolchain file, and any change to compiler flags or build types
decisions:
closes:     2026-08-13_adversarial-F09
blocks:
paused_by:
done:      CMakeLists.txt:7 deleted; no reference to toolchain.cmake remains outside this step file and the audit report.
                Fresh configure, paired: cmake -S . -B <scratch>/cfg-before and cfg-after, both with -DCMAKE_BUILD_TYPE=Release
                -DCMAKE_CXX_COMPILER_LAUNCHER=ccache, once with the line present and once without. Both exit 0, both select
                /usr/bin/c++ at GNU 13.3.0, and the two CMakeCache.txt files are byte-identical once the directory name is
                normalised. CMAKE_TOOLCHAIN_FILE reached the cache in neither run, which is what made the line dead.
                The accepts line is met against g++ 13.3.0, the compiler build/CMakeCache.txt already held (DEC-049). The body
                claim that a fresh configure picks AppleClang 16.0.0 is stale: it was written on the Apple machine this branch
                left at DEC-049, and no clang was involved here.
                Gate green: cmake --build build -j12 exit 0, ctest -L fast 9 of 9 in 16.54 s, clang-format.sh --check exit 0.
                No SPRT owed: build configuration cannot alter play, and the generated build system is byte-identical.
                DEV_MANUAL.md and MANUAL.md checked, no change needed; neither documents a CMake toolchain file.
                2026-08-13_adversarial-F09 stays planned until the audit is re-run.

## What is there

`CMakeLists.txt:7`:

```
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")
```

There is no `toolchain.cmake` in the tree — `git ls-files | grep -i toolchain`
returns `TOOLCHAIN.md` and nothing else. The line also sits after `project()`,
and CMake reads the toolchain file during the first `project()` call, so it
would do nothing even if the file existed. A fresh configure succeeds and picks
AppleClang 16.0.0 exactly as `build/` did.

## Why remove it rather than leave it

No effect today. The cost is that it reads as though the build were pinned to a
toolchain when it is not, and it is a trap with a delay on it: moving that line
above `project()`, or passing `cmake --toolchain`, turns a dead line into a
configure failure at the worst moment.

`TOOLCHAIN.md` documents tool setup and never mentions a CMake toolchain file,
so nothing in the project wants this.
author:    Maksym Bodnar
