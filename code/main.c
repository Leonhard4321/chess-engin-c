#include <stdio.h>
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>

#define WINDOW_SIZE 1400

enum {COLOR_WHITE, COLOR_BLACK, COLOR_BOTH};
enum {PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};

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
        int en_passant;        // Square index (0-63) if available, otherwise -1
        int castling_rights;   // 4 bits: White King/Queen side, Black King/Queen side
        int halfmove_clock;    // For the 50-move draw rule
        int fullmove_number;   // Incremented after Black's move
}Position;

typedef struct {
    int selectedSquare;
    bitboard drawPossableMoves; // Stores all the possable moves from the selectedSquare in the Bitboard format
} MouseState;

void handleMovement(MouseState *mouseState, Position *position);
void drawPosition(Position position, Texture2D pieceTex[2][6]);
void drawBoard(const Texture2D boardTex);
void drawPossableMoves(MouseState mouseState);

static int MousePosToSquare(Vector2 mousePosition);

Position startposition();

int main(void){

        InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Chess-Engin");
        SetTargetFPS(30);

        //Loading Textures
        const Texture2D boardTex = LoadTexture("assets/200.png");
        Texture2D pieceTex[2][6];
        pieceTex[COLOR_WHITE][PAWN] = LoadTexture("assets/wp.png");
        pieceTex[COLOR_WHITE][ROOK] = LoadTexture("assets/wr.png");
        pieceTex[COLOR_WHITE][KNIGHT] = LoadTexture("assets/wn.png");
        pieceTex[COLOR_WHITE][BISHOP] = LoadTexture("assets/wb.png");
        pieceTex[COLOR_WHITE][QUEEN] = LoadTexture("assets/wq.png");
        pieceTex[COLOR_WHITE][KING] = LoadTexture("assets/wk.png");
        pieceTex[COLOR_BLACK][PAWN] = LoadTexture("assets/bp.png");
        pieceTex[COLOR_BLACK][ROOK] = LoadTexture("assets/br.png");
        pieceTex[COLOR_BLACK][KNIGHT] = LoadTexture("assets/bn.png");
        pieceTex[COLOR_BLACK][BISHOP] = LoadTexture("assets/bb.png");
        pieceTex[COLOR_BLACK][QUEEN] = LoadTexture("assets/bq.png");
        pieceTex[COLOR_BLACK][KING] = LoadTexture("assets/bk.png");


        Position position = startposition();
        MouseState mouseState = {0};
        mouseState.selectedSquare = -1;

        while(!(WindowShouldClose())){
                handleMovement(&mouseState, &position);

                BeginDrawing();
                        ClearBackground(RAYWHITE);
        
                        drawBoard(boardTex);  
                        drawPossableMoves(mouseState);
                        drawPosition(position, pieceTex);
                EndDrawing();
        }

        UnloadTexture(boardTex);
        CloseWindow();

        return 0;   
}

void drawPossableMoves(MouseState mouseState){
        // Draw the highlighted square if a piece is selected 
        if(mouseState.selectedSquare < 0) return;
        Rectangle square;
        square.width = (float)(WINDOW_SIZE / 8);
        square.height = (float)(WINDOW_SIZE / 8);
        square.x = (mouseState.selectedSquare % 8) * (WINDOW_SIZE / 8);
        square.y = (int)((mouseState.selectedSquare) / 8) * (WINDOW_SIZE / 8);
        DrawRectangleRec(square, Fade(GREEN, 0.5f));

        // TDO: Draw the possible moves from the selected square using mouseState.drawPossableMoves bitboard

}

static int MousePosToSquare(Vector2 mousePosition) {
    int x = (int)(mousePosition.x / (WINDOW_SIZE / 8));
    int y = (int)(mousePosition.y / (WINDOW_SIZE / 8));
    return y * 8 + x;
}

int findPieceOnPosition(int square, Position position){
        for(int piece = 0; piece < 6; piece++){
                for(int color = 0; color < 2; color++){
                        if(position.pieces[color][piece] >> square & 1ULL){
                                // we found the piece on the square
                                return color * 6 + piece; 
                                // return the piece index, the order is: wp, wr, wn, wb, wq, wk, bp, br, bn, bb, bq, bk       
                        }
                }
        }
        return -1; // no piece found on the square
}

void handleMovement(MouseState *mouseState, Position *position) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    int square = MousePosToSquare(GetMousePosition());
    if (square < 0) return;

    if (mouseState->selectedSquare < 0) {
        if(!(position->occupancy[COLOR_BOTH] >> square & 1ULL)){return;} 
        mouseState->selectedSquare = square;
        // TODO: find all the possable moves from the selectedSquare and store them in mouseState->drawPossableMoves
    } else {
        int targetSquare = square;
        int piece1 = findPieceOnPosition(mouseState->selectedSquare, *position);
        int piece2 = findPieceOnPosition(targetSquare, *position);

        // update the position by moving the piece from selectedSquare to targetSquare
        position->pieces[piece1 / 6][piece1 % 6] &= ~(1ULL << mouseState->selectedSquare); // remove the piece from the original square
        position->pieces[piece1 / 6][piece1 % 6] |= (1ULL << targetSquare); // place the piece on the target square
        if(piece2 != -1){
                // if there is a piece on the target square, remove it
                position->pieces[piece2 / 6][piece2 % 6] &= ~(1ULL << targetSquare);
        }

        // update the occupancy bitboards
        position->occupancy[COLOR_BOTH] &= ~(1ULL << mouseState->selectedSquare); // remove the piece from the original square
        position->occupancy[COLOR_BOTH] |= (1ULL << targetSquare); // place the piece on the target square
        position->occupancy[position->side_to_move] &= ~(1ULL << mouseState->selectedSquare); // remove the piece from the original square
        position->occupancy[position->side_to_move] |= (1ULL << targetSquare); // place the piece on the target square
        if(piece2 != -1){
                // if there is a piece on the target square, remove it from the occupancy bitboard of the opponent
                position->occupancy[1 - position->side_to_move] &= ~(1ULL << targetSquare);
        }
        
        // deselect the piece after moving
        mouseState->selectedSquare = -1; 
    }
}

void drawPosition(Position position, Texture2D pieceTex[2][6]){
        const int squareSize = WINDOW_SIZE / 8;
        Rectangle destRect = {0, 0, (float)squareSize, (float)squareSize};
        const Vector2 pos = {0, 0};
        
        for(int piece = 0; piece < 6; piece++){
                for(int color = 0; color < 2; color++){
                        for(int i = 0; i < 64; i++){
                                if(position.pieces[color][piece] >> i & 1ULL){
                                        const Rectangle sourceRect = {0, 0, (float)pieceTex[color][piece].width, (float)pieceTex[color][piece].height};
                                        destRect.x = (i % 8) * squareSize;
                                        destRect.y = (int)(i / 8) * squareSize;
                                        DrawTexturePro(pieceTex[color][piece], sourceRect, destRect, pos, 0, WHITE);
                                }
                        }
                }
        }
}

Position startposition(){
        Position position = {0};
        position.castling_rights = 0x0000F;
        position.en_passant = -1;
        position.fullmove_number = 0;
        position.halfmove_clock = 0;
        position.occupancy[COLOR_WHITE] =       0xFFFF000000000000;
        position.occupancy[COLOR_BLACK] =       0x000000000000FFFF;
        position.occupancy[COLOR_BOTH]  =       0xFFFF00000000FFFF;
        position.pieces[COLOR_WHITE][PAWN] =    0x00FF000000000000;
        position.pieces[COLOR_BLACK][PAWN] =    0x000000000000FF00;
        position.pieces[COLOR_WHITE][ROOK] =    0x8100000000000000;
        position.pieces[COLOR_BLACK][ROOK] =    0x0000000000000081;
        position.pieces[COLOR_WHITE][KNIGHT] =  0x4200000000000000;
        position.pieces[COLOR_BLACK][KNIGHT] =  0x0000000000000042;
        position.pieces[COLOR_WHITE][BISHOP] =  0x2400000000000000;
        position.pieces[COLOR_BLACK][BISHOP] =  0x0000000000000024;
        position.pieces[COLOR_WHITE][QUEEN] =   0x0800000000000000;
        position.pieces[COLOR_BLACK][QUEEN] =   0x0000000000000008;
        position.pieces[COLOR_WHITE][KING] =    0x1000000000000000;
        position.pieces[COLOR_BLACK][KING] =    0x0000000000000010;
        position.side_to_move = COLOR_WHITE;

        return position;
}

void drawBoard(const Texture2D boardTex){
        
        const Rectangle sourceRect = {0, 0, (float)boardTex.width, (float)boardTex.height};
        const Rectangle destRect = {0, 0, WINDOW_SIZE, WINDOW_SIZE};
        const Vector2 pos = {0, 0};
        const Color color = {255, 255, 255, 255};
        DrawTexturePro(boardTex, sourceRect, destRect, pos, 0, color);
}