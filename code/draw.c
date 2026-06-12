#include "main.h"

void drawPossableMoves(MouseState mouseState){
        // Draw the highlighted square if a piece is selected 
        if(mouseState.selectedSquare < 0) return;
        Rectangle square;
        square.width = (float)(WINDOW_SIZE / 8);
        square.height = (float)(WINDOW_SIZE / 8);
        square.x = (mouseState.selectedSquare % 8) * (WINDOW_SIZE / 8);
        square.y = (int)((mouseState.selectedSquare) / 8) * (WINDOW_SIZE / 8);
        DrawRectangleRec(square, Fade(GREEN, 0.5f));

        // Draw the possible moves from the selected square
        for(int i = 0; i < 64; i++){
                if(mouseState.drawPossableMoves >> i & 1ULL){
                        Rectangle moveSquare;
                        moveSquare.width = (float)(WINDOW_SIZE / 8);
                        moveSquare.height = (float)(WINDOW_SIZE / 8);
                        moveSquare.x = (i % 8) * (WINDOW_SIZE / 8);
                        moveSquare.y = (int)(i / 8) * (WINDOW_SIZE / 8);
                        DrawRectangleRec(moveSquare, Fade(BLUE, 0.5f));
                }
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

void drawBoard(const Texture2D boardTex){
        
        const Rectangle sourceRect = {0, 0, (float)boardTex.width, (float)boardTex.height};
        const Rectangle destRect = {0, 0, WINDOW_SIZE, WINDOW_SIZE};
        const Vector2 pos = {0, 0};
        const Color color = {255, 255, 255, 255};
        DrawTexturePro(boardTex, sourceRect, destRect, pos, 0, color);
}