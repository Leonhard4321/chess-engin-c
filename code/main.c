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
        MouseState mouseState = {0};
        mouseState.selectedSquare = -1;
        mouseState.checkedSquare = -1;

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

int MousePosToSquare(Vector2 mousePosition) {
        int x = (int)(mousePosition.x / (WINDOW_SIZE / 8));
        int y = (int)(mousePosition.y / (WINDOW_SIZE / 8));
        return y * 8 + x;
}

void handleMovement(MouseState *mouseState, Position *position) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

        int square = MousePosToSquare(GetMousePosition());
        if (square < 0) return;

        if (mouseState->selectedSquare < 0) {
                if(!(position->occupancy[position->side_to_move] >> square & 1ULL)){return;} 
                mouseState->selectedSquare = square;
                mouseState->drawPossableMoves = legalMoves(*position, square);
        } else {
                int targetSquare = square;

                if(targetSquare == mouseState->selectedSquare){ // if the player clicks on the selected square, deselect it
                        mouseState->selectedSquare = -1;
                        return;
                }
                if(!(mouseState->drawPossableMoves >> targetSquare & 1ULL)){ // if the player clicks on a square that is not a legal move, do nothing
                        return;
                }

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

                // if the player captures en passant, remove the captured pawn (only for pawn moves)
                if(piece1 % 6 == PAWN && position->en_passant == targetSquare){ 
                        if(position->side_to_move == COLOR_WHITE){
                                position->pieces[COLOR_BLACK][PAWN] &= ~(1ULL << (targetSquare + 8)); 
                                position->occupancy[COLOR_BLACK] &= ~(1ULL << (targetSquare + 8));
                                position->occupancy[COLOR_BOTH] &= ~(1ULL << (targetSquare + 8)); 
                        } else {
                                position->pieces[COLOR_WHITE][PAWN] &= ~(1ULL << (targetSquare - 8)); 
                                position->occupancy[COLOR_WHITE] &= ~(1ULL << (targetSquare - 8)); 
                                position->occupancy[COLOR_BOTH] &= ~(1ULL << (targetSquare - 8));
                        }
                }

                // update the en passant square if a pawn moves two squares forward and look for promotion
                if(piece1 % 6 == PAWN){
                        if(mouseState->selectedSquare - targetSquare == 16){
                                // if the pawn moves two squares forward, set the en passant square
                                position->en_passant = mouseState->selectedSquare - 8;
                        } else if(mouseState->selectedSquare - targetSquare == -16){
                                // if the pawn moves two squares forward, set the en passant square
                                position->en_passant = mouseState->selectedSquare + 8;
                        } else {
                                // otherwise, reset the en passant square
                                position->en_passant = -1;
                        }
                        if((targetSquare / 8 == 0 ) || (targetSquare / 8 == 7 )){
                                // if the pawn reaches the last rank, promote it to a queen
                                char promotionPieceChar;
                                int promotionPieceInt;
                                printf("select piece to promote to:\nn: knight\nb: bishop\nr: rook\nq: queen\n");
                                scanf(" %c", &promotionPieceChar);
                                switch(promotionPieceChar){
                                        case 'q': promotionPieceInt = QUEEN; break;
                                        case 'r': promotionPieceInt = ROOK; break;
                                        case 'b': promotionPieceInt = BISHOP; break;
                                        case 'n': promotionPieceInt = KNIGHT; break;
                                }
                                position->pieces[position->side_to_move][PAWN] &= ~(1ULL << targetSquare); // remove the pawn from the target square
                                position->pieces[position->side_to_move][promotionPieceInt] |= (1ULL << targetSquare); // place the promoted piece on the target square
                        }
                }

                // deselect the piece after moving
                mouseState->selectedSquare = -1;
                position->side_to_move = 1 - position->side_to_move; // switch the side to move
        }
}