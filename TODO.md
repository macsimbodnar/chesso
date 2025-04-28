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

* Crash with position:
  position startpos moves d2d4 d7d5 c2c4 d5c4 g1f3 b7b5 a2a4 c7c6 a4b5 c6b5 e2e3 g8f6 b2b3 a7a5 b3c4 b5b4 f3e5 c8b7 c4c5 f6d7 f1b5 b7g2 e5d7 b8d7 h1g1 g2b7 c5c6 b7c6 b5c6 a8a6 c6b5 a6g6 g1g6 h7g6 b5d7 e8d7
