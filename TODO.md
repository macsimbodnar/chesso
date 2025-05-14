# Todo list for chesso

* Try again to integrate the Aspiration windows
* Implement TT
* Move repetition draw
* Perpetual checks
* Insufficient material draw
* Add if is in check to the board. Make move should set it and should be used during next move generation. Also form fen should set it. And whenever the board is created.
* Solve the `is_pin` and `generate_attacks_vector` const board but it's modified
* Make it compile and work in windows

## Bugs

* "8/3P4/8/8/6k1/8/8/3K4 w - - 0 1" position produce different moves in case of different depth

* "r1b1kbnr/pp1p1ppp/4p3/4P3/3pq3/6P1/PPP1B2P/RNBQK2R b KQkq - 1 8" this position with TT enabled generates illegal moves: bestmove e4h1 ponder d1c2

* Still draw in 3 fold repetition some times

* Still issues with illegal PV:
  Illegal PV move g3f2 from chesso_engine (8)
  PV: Kg6 Kh3 Kh5 Kg3 Kg6 g3f2
  Illegal PV move g3h4 from chesso_engine (8)
  PV: Rb2 g3h4
  Illegal PV move g3h4 from chesso_engine (8)
  PV: Rb2 g3h4

* Soe time can make an illegal move (--). Make sure to always have a backup move, even if random.

* Change the == move_t overload operator to check ALL the parameters, and use a function to soft compare.
* Connected rooks score

* BUG: 
 >Chesso Bitboard(1): position startpos moves d2d4 d7d6 e2e4 g8f6 b1c3 b8d7 g2g4 h7h6 c1e3 e7e5 h2h3 c7c6 g1e2 b7b5 a2a3 c8b7 f1g2 a7a5 e2g3 f6g8 e1g1 d8c7 d1e2 a8d8 d4d5 c7b8 b2b4 a5b4 a3b4
>Chesso Bitboard(1): isready
<Chesso Bitboard(1): readyok
>Chesso Bitboard(1): go wtime 227619 btime 251662 movestogo 26
<Chesso Bitboard(1): info score cp 410 time 0 depth 1 nodes 26 pv b4c3 
<Chesso Bitboard(1): info score cp 390 time 0 depth 2 nodes 100 pv b4c3 a1d1 
<Chesso Bitboard(1): info score cp 400 time 0 depth 3 nodes 220 pv b4c3 d5c6 b7c6 
<Chesso Bitboard(1): info score cp 380 time 0 depth 4 nodes 948 pv b4c3 d5c6 b7c6 a1d1 
<Chesso Bitboard(1): info score cp 410 time 1 depth 5 nodes 4225 pv b4c3 d5c6 b7c6 a1d1 c6b7 
<Chesso Bitboard(1): info score cp 385 time 3 depth 6 nodes 9365 pv b4c3 d5c6 b7c6 a1d1 b8c7 g3f5 
<Chesso Bitboard(1): info score cp 310 time 11 depth 7 nodes 38883 pv b4c3 d5c6 b7c6 a1a3 d7f6 a3c3 c6b7 
<Chesso Bitboard(1): info score cp 270 time 34 depth 8 nodes 112568 pv b4c3 d5c6 b7c6 a1a3 d7f6 a3c3 c6a8 g3f5 
<Chesso Bitboard(1): info score cp 305 time 106 depth 9 nodes 337834 pv b4c3 d5c6 b7c6 e2d3 d7f6 d3c3 c6b7 a1d1 f8e7 
<Chesso Bitboard(1): info score cp 300 time 330 depth 10 nodes 970257 pv b4c3 d5c6 b7c6 e3a7 b8c7 e2d3 c6a8 d3b5 f8e7 a1d1 
<Chesso Bitboard(1): info score cp 285 time 1507 depth 11 nodes 4199099 pv b4c3 d5c6 b7c6 e2d3 d7f6 d3c3 c6b7 e3b6 d8d7 a1d1 f8e7 
<Chesso Bitboard(1): bestmove b4c3 ponder d5c6

best move here is illegal
