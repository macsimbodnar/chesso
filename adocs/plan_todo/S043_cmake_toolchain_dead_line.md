id:         S043
goal:       delete the CMAKE_TOOLCHAIN_FILE line that names a file that does not exist
accepts:    a fresh configure in a clean directory succeeds and selects the same compiler as before; no reference to toolchain.cmake remains
touches:    CMakeLists.txt
excludes:   introducing an actual toolchain file, and any change to compiler flags or build types
decisions:
closes:     2026-08-13_adversarial-F09
blocks:
paused_by:
done:

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
