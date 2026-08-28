# main project

A hobby chess engine and graphical board I built in C using raylib graphics library, `raylib`. 

I started this project to get more better with C and to learn how chess programming actually works under the hood. It features full legal move generation, all standard Chess-Rules, and a custom bot that uses negamax search with alpha-beta pruning. It is not going to beat Magnus Carlsen (the best Chessplayer alive), but it plays a solid game and will dynamically adjust its search depth based on positions to analize. 

## what it features

- Fully playable visuals with the raylib library. 
- Standard chess rules: Handles castling, en passant, pawn promotion, stalemate, checkmate, and threefold repetition.
- Custom Engine: Uses negamax search with alpha-beta pruning and basic move ordering

## what you need

To build and run this project, you'll need:
- Preferable a Linux system, for all other systems there might be some thinkering to be done
- A C compiler (like GCC)
- `raylib`  raylib installed on your system

## how to run

From the project root directory, compile the game using:

```bash
make
./main