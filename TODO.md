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
