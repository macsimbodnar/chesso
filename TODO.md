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
