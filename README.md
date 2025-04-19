# Chesso

```bash
                                                       .::.
                                            _()_       _::_
                                  _O      _/____\_   _/____\_
           _  _  _     ^^__      / //\    \      /   \      /
          | || || |   /  - \_   {     }    \____/     \____/
          |_______| <|    __<    \___/     (____)     (____)
    _     \__ ___ / <|    \      (___)      |  |       |  |
   (_)     |___|_|  <|     \      |_|       |__|       |__|
  (___)    |_|___|  <|______\    /   \     /    \     /    \
  _|_|_    |___|_|   _|____|_   (_____)   (______)   (______)
 (_____)  (_______) (________) (_______) (________) (________)
 /_____\  /_______\ /________\ /_______\ /________\ /________\
 ```

## How to

### Clone the repo

```bash
git clone git@github.com:macsimbodnar/chesso.git
cd chesso
git submodule update --init --recursive
```

### Install dependencies for the tests

```bash
sudo apt install build-essential

# SDL2 is required for the test gui application
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-gfx-dev

# Optional: Install ninja build system
sudo apt install ninja-build
```

### Build

```bash
# From the project root directory
mkdir build
cd build

# If you use Ninja
cmake ..  -GNinja -DCMAKE_BUILD_TYPE=Release
ninja

# If you use make
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
```

### Run the tests

```bash
# After you build the repo. From the build directory
ctest --verbose
```

### Run the test gui application

```bash
# After you build the repo. From the build directory
cd tests
./test_gui
```

### Build for profiler

```bash
cmake ..  -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg
ninja

# Execute the program in order to generate the gmon.out file
cd test
./debug_main

# Get profiler results
gprof debug_main gmon.out > analysis.txt
```

## Mentions

[Principal Variation Search](https://web.archive.org/web/20071030220825/http://www.brucemo.com/compchess/programming/pvs.htm)
[Late Move Reduction](https://web.archive.org/web/20150212051846/http://www.glaurungchess.com/lmr.html)
