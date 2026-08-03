# chess-engin-c
A graphical chess engine written in C using `raylib`.

This project includes a playable chess board, legal move generation, chess rules enforcement, and a simple engine that plays against you with adaptive search depth.

## Features

- 2D chess board rendered with `raylib`
- Player vs Player mode
- Player vs Computer mode
- Move generation for all standard chess pieces
- Pawn promotion, castling, en passant, stalemate, checkmate, and threefold repetition draw detection
- Engine using negamax search with alpha-beta pruning and quiescence search
- Adaptive search depth based on board complexity

## Requirements

- GCC or another C compiler
- `raylib` library installed
- `make` command available

## Build

From the project root:

```bash
make start
```

This compiles the source files and produces the executable `main`.

## Run

After building, run:

```bash
./main
```

## Game Modes

The current mode is set in `code/main.h` with the `PLAYING_STYLE` macro:

- `PVP` — Player vs Player
- `PVBOT` — Player vs Computer
- `BOTVBOT` — Computer vs Computer

You can also set `PLAYER_COLOR` in `code/main.h` to choose whether the human player uses white or black in `PVBOT` mode.

## Controls

- Left click to select a piece
- Left click again to move it to a legal target square
- Pawn promotion is selected by typing one of: `n`, `b`, `r`, or `q`

## Notes

- The engine chooses moves using move ordering heuristics and evaluates positions by material balance.
- The board textures are stored in `assets/` and loaded at runtime.
- The game window is `800x800` pixels by default, but can be adjusted, if need be.

## Project Structure

- `code/main.c` — main game loop and input handling
- `code/main.h` — data structures, macros, and shared declarations
- `code/bot.c` — engine search, evaluation, and bot move selection
- `code/findLegalMoves.c` — legal move generation and move application
- `code/setup.c` — position initialization and helpers
- `code/draw.c` — rendering functions
- `assets/` — board and piece textures

## License

This project is licensed under the MIT License. See `LICENSE` for details.