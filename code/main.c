#include "main.h"

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
        position.castling_rights = 0x0F; // all sides are allowed to castle
        MouseState mouseState = {0};
        mouseState.selectedSquare = -1;
        int gameOver = 0;
        
        // Position history for threefold repetition
        Position positionHistory[256];
        int historyCount = 1;
        positionHistory[0] = position;

        if(PLAYING_STYLE != PVP){
                setupBot();
        }

        while(!(WindowShouldClose())){
                int madeMove = 0;

                if(PLAYING_STYLE == PVP){
                        madeMove = handleMovement(&mouseState, &position, &gameOver);   
                }else if(PLAYING_STYLE == PVBOT){
                        if(position.side_to_move == PLAYER_COLOR){
                                madeMove = handleMovement(&mouseState, &position, &gameOver); 
                        }else{
                                madeMove = handleBot(&position, &gameOver, positionHistory, historyCount);
                        }
                }else if(PLAYING_STYLE == BOTVBOT){

                }
                
                // Check for threefold repetition after an actual move
                if(madeMove && gameOver == STATE_ONGOING){
                        int repetitionCount = 0;
                        for(int i = 0; i < historyCount; i++){
                                if(positionHistory[i].pieces[0][0] == position.pieces[0][0] &&
                                   positionHistory[i].pieces[0][1] == position.pieces[0][1] &&
                                   positionHistory[i].pieces[0][2] == position.pieces[0][2] &&
                                   positionHistory[i].pieces[0][3] == position.pieces[0][3] &&
                                   positionHistory[i].pieces[0][4] == position.pieces[0][4] &&
                                   positionHistory[i].pieces[0][5] == position.pieces[0][5] &&
                                   positionHistory[i].pieces[1][0] == position.pieces[1][0] &&
                                   positionHistory[i].pieces[1][1] == position.pieces[1][1] &&
                                   positionHistory[i].pieces[1][2] == position.pieces[1][2] &&
                                   positionHistory[i].pieces[1][3] == position.pieces[1][3] &&
                                   positionHistory[i].pieces[1][4] == position.pieces[1][4] &&
                                   positionHistory[i].pieces[1][5] == position.pieces[1][5] &&
                                   positionHistory[i].side_to_move == position.side_to_move){
                                repetitionCount++;
                                }
                        }
                        if(repetitionCount >= 3){
                                gameOver = STATE_THREEFOLD;
                        }
                }
                
                // Add position to history after a move
                if(madeMove && gameOver == STATE_ONGOING && historyCount < 256){
                        positionHistory[historyCount++] = position;
                }

                BeginDrawing();
                        ClearBackground(RAYWHITE);
        
                        drawBoard(boardTex);  
                        drawPossableMoves(mouseState);
                        drawPosition(position, pieceTex);
                        
                        if(gameOver == STATE_CHECKMATE || gameOver == STATE_STALEMATE || gameOver == STATE_THREEFOLD){
                                if(gameOver == STATE_STALEMATE){
                                        DrawText("Game is a DRAW (Stalemate) - Close window to exit", 20, 20, 20, RED);
                                }else if(gameOver == STATE_THREEFOLD){
                                        DrawText("Game is a DRAW (Threefold Repetition) - Close window to exit", 20, 20, 20, RED);
                                }else if(gameOver == STATE_CHECKMATE){
                                        if(position.side_to_move == COLOR_BLACK){
                                                DrawText("WHITE won the game - Close window to exit", 20, 20, 20, RED);
                                        }else{
                                                DrawText("BLACK won the game - Close window to exit", 20, 20, 20, RED);
                                        }
                                }
                        }
                EndDrawing();
        }

        printf("-----------------------------\n     ");
        if(gameOver == STATE_STALEMATE){
                printf("   Game is a DRAW (Stalemate)\n");
        }else if(gameOver == STATE_THREEFOLD){
                printf("   Game is a DRAW (Threefold Repetition)\n");
        }else if(gameOver == STATE_CHECKMATE){
                if(position.side_to_move == COLOR_BLACK){
                        printf("WHITE won the game\n");
                }else{
                        printf("BLACK won the game\n");
                }
        }
        printf("-----------------------------\n");
        return 0;   
}

int MousePosToSquare(Vector2 mousePosition) {
        int x = (int)(mousePosition.x / (WINDOW_SIZE / 8));
        int y = (int)(mousePosition.y / (WINDOW_SIZE / 8));
        return y * 8 + x;
}

int handleMovement(MouseState *mouseState, Position *position, int *gameOver) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return 0;

        int square = MousePosToSquare(GetMousePosition());
        if (square < 0) return 0;

        if (mouseState->selectedSquare < 0) {
                if(!(position->occupancy[position->side_to_move] >> square & 1ULL)){return 0;} 
                mouseState->selectedSquare = square;
                mouseState->drawPossableMoves = legalMoves(*position, square);
        } else {
                int targetSquare = square;

                if(targetSquare == mouseState->selectedSquare){ // if the player clicks on the selected square, deselect it
                        mouseState->selectedSquare = -1;
                        return 0;
                }
                if(!(mouseState->drawPossableMoves >> targetSquare & 1ULL)){ // if the player clicks on a square that is not a legal move, do nothing
                        return 0;
                }


                // check if the movement was a promotion
                int selectedPieceType;
                char promotionPieceChar;
                int promotionPieceInt = QUEEN;
                findPieceOnPosition(mouseState->selectedSquare, *position, NULL, &selectedPieceType);
                int targetRank = targetSquare / 8;

                int isPromotion = (selectedPieceType == PAWN) && 
                           ((position->side_to_move == COLOR_WHITE && targetRank == 0) || 
                            (position->side_to_move == COLOR_BLACK && targetRank == 7));

                if(isPromotion){
                        printf("select piece to promote to:\nn: knight\nb: bishop\nr: rook\nq: queen\n");
                        scanf(" %c", &promotionPieceChar);
                        switch(promotionPieceChar){
                                case 'q': promotionPieceInt = QUEEN; break;
                                case 'r': promotionPieceInt = ROOK; break;
                                case 'b': promotionPieceInt = BISHOP; break;
                                case 'n': promotionPieceInt = KNIGHT; break;
                        }                         
                }


                // apply the choosen move to the position
                *position = makeMove(mouseState->selectedSquare, targetSquare, *position, promotionPieceInt);
                // look for a checkmate or draw
                *gameOver = gameState(*position);
                // deselect the piece after moving
                mouseState->selectedSquare = -1;
                return 1;
        }

        return 0;
}


