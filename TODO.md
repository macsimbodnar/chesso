# Todo list for chesso

* Move repetition draw
* Perpetual checks
* Insufficient material draw
* Add if is in check to the board. Make move should set it and should be used during next move generation. Also form fen should set it. And whenever the board is created.
* Solve the `is_pin` and `generate_attacks_vector` const board but it's modified
* Make it compile and work in windows

## Bugs

* "8/3P4/8/8/6k1/8/8/3K4 w - - 0 1" position produce different moves in case of different depth
