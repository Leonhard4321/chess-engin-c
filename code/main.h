#pragma once
#include <stdio.h>
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>

#define WINDOW_SIZE 800


enum {PVP, PVBOT, BOTVBOT};
#define PLAYING_STYLE PVBOT // You can choose your modes here
// PVP:     Player vs Player
// PVBOT:   Player vs PC
// BOTVBOT: PC vs PC

#define PLAYER_COLOR COLOR_WHITE // This is only relevent in PVBOT

enum {COLOR_WHITE, COLOR_BLACK, COLOR_BOTH};
enum {PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
enum GameState {STATE_ONGOING, STATE_CHECK, STATE_STALEMATE, STATE_CHECKMATE, STATE_THREEFOLD};

typedef uint64_t bitboard;

typedef struct{

        bitboard pieces[2][6]; 
        //in pieces you get to quickly look up where a specific piece type stands on the board
        //the order is: PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
        //              COLOR_WHITE, COLOR_BLACK

        bitboard occupancy[3]; // [WHITE], [BLACK], [BOTH]
        //in the occupancy you get to quickly look up if any piece is anywhere 
        //you can filte the pieces only by color, the order goes as follows:
        //COLOR_WHITE, COLOR_BLACK, COLOR_BOTH

        //metadata
        int side_to_move;
        int en_passant;        // square where en passant capture is possible, otherwise -1
        int castling_rights;   // 4 bits: White King/Queen side, Black King/Queen side, 1 means you have the castlingright, 0 means you don't have it
        int halfmove_clock;    // For the 50-move draw rule
        int fullmove_number;   // Incremented after Black's move
}Position;

typedef struct {
    int selectedSquare;
    bitboard drawPossableMoves; // Stores all the possible moves from the selectedSquare in the Bitboard format
} MouseState;

Position startposition();

int handleMovement(MouseState *mouseState, Position *position, int *gameState);
int handleBot(Position *position, int *gameState, Position *positionHistory, int historyCount);
void setupBot();
void drawPosition(Position position, Texture2D pieceTex[2][6]);
void drawBoard(const Texture2D boardTex);
void drawPossableMoves(MouseState mouseState);

int MousePosToSquare(Vector2 mousePosition);
bitboard legalMoves(Position position, int square);

Position makeMove(int selectedSquare, int targetedSquare, Position currentPosition, int promotionPiece);
int gameState(Position position);

int findPieceOnPosition(int square, Position position, int* pieceColor, int* pieceType);

bitboard legalMoves(Position position, int square);