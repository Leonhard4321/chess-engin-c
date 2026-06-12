#pragma once
#include <stdio.h>
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>

#define WINDOW_SIZE 1400

enum {COLOR_WHITE, COLOR_BLACK, COLOR_BOTH};
enum {PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
enum {EN_PASSANT};

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
        int castling_rights;   // 4 bits: White King/Queen side, Black King/Queen side
        int halfmove_clock;    // For the 50-move draw rule
        int fullmove_number;   // Incremented after Black's move
}Position;

typedef struct {
    int selectedSquare;
    int checkedSquare; // the square on which the king is being checked, if not in check then -1
    bitboard drawPossableMoves; // Stores all the possible moves from the selectedSquare in the Bitboard format
} MouseState;

void handleMovement(MouseState *mouseState, Position *position);
void drawPosition(Position position, Texture2D pieceTex[2][6]);
void drawBoard(const Texture2D boardTex);
void drawPossableMoves(MouseState mouseState);

int MousePosToSquare(Vector2 mousePosition);
int findPieceOnPosition(int square, Position position);
int isKingCheck(Position position);

Position startposition();

bitboard legalMoves(Position position, int square);