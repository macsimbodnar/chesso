# Todo and Bugs list

## Todo list

### Evaluation

* Add doubled pawns penalty
* Isolated pawn penalty
* Passed pawn bonus
* Bishop mobility bonus
* Rook open file bonus
* Rook semi open file bonus
* Queen mobility
* King semi open file penalty
* King open file penalty
* King safety bonus
* Connected rook bonus
* Optimize order captures

### Search

* Perpetual checks
* Insufficient material draw
* Experiment with Razoring
* Generate only captures in quiescence

### Board

* Generate legal moves
* Optimize move generation
* Generate only capture moves for quiescence

### Generic

* Move critical code to inline functions
* Make it compile and work in windows

## Bugs

* In the evaluate move function handle the en-passant
* The engine stack in this search:
  >Chesso Bitboard(1): position startpos moves g1f3 d7d5 d2d4 g8f6 c2c4 e7e6 b1c3 c7c5 c4d5 f6d5 e2e4 d5c3 b2c3 c5d4 c3d4 f8b4 c1d2 b4d2 d1d2 e8g8 a1d1 b7b6 f1e2 b8d7 e4e5 c8b7 e1g1 a8c8 d1c1 c8c1 d2c1 f8e8 f1e1 d7f8 c1b2 f8g6 e2b5 e8e7 e1c1 b7f3 g2f3 f7f6 b5a6 e7c7 c1c7 d8c7 a6d3 g6h4 d3e2 f6e5 d4e5 h4g6 f3f4 g6f4 e2f3 g7g6 g1f1 c7c5 f3g2 f4d3 b2d2 d3e5 f1g1 b6b5 h2h4 g8g7 g1h2 a7a5 h2h1 e5c4 d2d7 g7h6 d7e6 c5f2 e6e7 f2f5 h1g1 a5a4 g1h1 a4a3 h1h2 f5f4 h2h3 c4e3 e7e4 e3g2 e4g2 b5b4 g2e2 f4f5 h3h2 f5f6 h2h3 h6g7 h3g4 h7h5 g4h3 b4b3 a2b3 f6c3 h3g2 c3b2 g2f1 b2e2 f1e2 a3a2 b3b4 a2a1q e2d3 a1f6 d3c4 f6h4 c4b3 h4g5 b3a3 h5h4 a3a4 h4h3 a4a3 h3h2 a3b3 h2h1q
  >Chesso Bitboard(1): isready
  <Chesso Bitboard(1): readyok
  >Chesso Bitboard(1): go depth 9
  <Chesso Bitboard(1): info score cp -2022 time 0 depth 1 nodes 13 pv b3b2
  <Chesso Bitboard(1): info score cp -2029 time 0 depth 2 nodes 450 pv b3b2 g5f6 b2b3
  <Chesso Bitboard(1): info score cp -2053 time 1 depth 3 nodes 3341 pv b3a3 g5e3 a3b2 h1c1 b2a2
  <Chesso Bitboard(1): info score mate -4 time 7 depth 4 nodes 35536 pv b3b2 g5d2 b2b3 h1d5 b3a3 d5a2
  <Chesso Bitboard(1): info score mate -4 time 19 depth 5 nodes 100188 pv b3b2 h1d5 b4b5 g5d2 b2b1 d5a2
  <Chesso Bitboard(1): info score mate -4 time 133 depth 6 nodes 669582 pv b3b2 h1d5 b4b5 g5d2 b2a3 d5a2
  <Chesso Bitboard(1): info score mate -4 time 1629 depth 7 nodes 7456262 pv b3b2 h1d5 b2a3 d5c4 a3a4 c4a2
  <Chesso Bitboard(1): info score mate -4 time 35350 depth 8 nodes 182277694 pv b3b2 h1d5 b4b5 d5c4 b2b1 g5c1
