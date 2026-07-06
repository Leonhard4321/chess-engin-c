#include "main.h"

bitboard legalMovesPawn(Position position, int square, int color); // is the only piece that need color to figur out possable moves
bitboard legalMovesRook(Position position, int square);
bitboard legalMovesKnight(Position position, int square);
bitboard legalMovesBishop(Position position, int square);
bitboard legalMovesQueen(Position position, int square);
bitboard legalMovesKing(Position position, int square);

bitboard pseudoLegalMoves(Position position, int square);
bitboard pseudoLegalMovesKing(Position position, int square);

int isSquareAttacked(Position position, int square, int attackerColor);
int getKingSquare(Position position, int color);
int isMoveLegal(Position currentPosition, int startSquare, int targetSquare);

bitboard legalMoves(Position position, int square){
        bitboard moves = pseudoLegalMoves(position, square);
        for(int i = 0; i < 64; i++){
                if(!(moves & (1ULL << i))){continue;}
                if((isMoveLegal(position, square, i))){
                        moves &= ~(1ULL << i);
                }
        }
        return moves;
}

bitboard pseudoLegalMoves(Position position, int square){
        int piece;
        int color;
        int occupancy = findPieceOnPosition(square, position, &color, &piece);

        if(occupancy == -1) return 0; // no piece on the square

        if(piece == PAWN){
                return legalMovesPawn(position, square, color);        
        }
        if(piece == ROOK){
                return legalMovesRook(position, square);
        }
        if(piece == KNIGHT){
                return legalMovesKnight(position, square);
        }
        if(piece == BISHOP){
                return legalMovesBishop(position, square);
        }
        if(piece == QUEEN){
                return legalMovesQueen(position, square);
        }
        if(piece == KING){
                return legalMovesKing(position, square);
        }        

        return 0x0123456789ABCDEF; // this part of the funtkion should never be reached
}

// generates a new position in which a piece has moved from square a to b
Position makeMove(int selectedSquare, int targetedSquare, Position currentPosition, int promotionPiece){

        Position newPosition = currentPosition;

        int selectedPieceColor;
        int selectedPieceType;
        int targetedPieceColor;
        int targetedPieceType;
        findPieceOnPosition(selectedSquare, currentPosition, &selectedPieceColor, &selectedPieceType); // we don't care if the selected square is occupied or not
        int isTargetedSquareOccupied = findPieceOnPosition(targetedSquare, currentPosition, &targetedPieceColor, &targetedPieceType);

        // update the position by moving the piece from selectedSquare to targetSquare
        newPosition.pieces[selectedPieceColor][selectedPieceType] &= ~(1ULL << selectedSquare); // remove the piece from the original square
        newPosition.pieces[selectedPieceColor][selectedPieceType] |= (1ULL << targetedSquare); // place the piece on the target square
        if(isTargetedSquareOccupied == 0){
                // if there is a piece on the target square, remove it
                newPosition.pieces[targetedPieceColor][targetedPieceType] &= ~(1ULL << targetedSquare);
                // set the clock for 50 move rule to 0
                newPosition.halfmove_clock = 0;
        }

        // update the occupancy bitboards
        newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << selectedSquare); // remove the piece from the original square
        newPosition.occupancy[COLOR_BOTH] |= (1ULL << targetedSquare); // place the piece on the target square
        newPosition.occupancy[currentPosition.side_to_move] &= ~(1ULL << selectedSquare); // remove the piece from the original square
        newPosition.occupancy[currentPosition.side_to_move] |= (1ULL << targetedSquare); // place the piece on the target square
        if(isTargetedSquareOccupied == 0){
                // if there is a piece on the target square, remove it from the occupancy bitboard of the opponent
                newPosition.occupancy[1 - currentPosition.side_to_move] &= ~(1ULL << targetedSquare);
        }

        // if the player captures en passant, remove the captured pawn (only for pawn moves)
        if(selectedPieceType == PAWN && currentPosition.en_passant == targetedSquare){ 
                if(currentPosition.side_to_move == COLOR_WHITE){
                        newPosition.pieces[COLOR_BLACK][PAWN] &= ~(1ULL << (targetedSquare + 8)); 
                        newPosition.occupancy[COLOR_BLACK] &= ~(1ULL << (targetedSquare + 8));
                        newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << (targetedSquare + 8)); 
                } else {
                        newPosition.pieces[COLOR_WHITE][PAWN] &= ~(1ULL << (targetedSquare - 8)); 
                        newPosition.occupancy[COLOR_WHITE] &= ~(1ULL << (targetedSquare - 8)); 
                        newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << (targetedSquare - 8));
                }
                // set the clock for 50 move rule to 0
                newPosition.halfmove_clock = 0;
        }

        // update the en passant square if a pawn moves two squares forward and look for promotion
        if(selectedPieceType == PAWN){
                if(selectedSquare - targetedSquare == 16){
                        // if the pawn moves two squares forward, set the en passant square
                        newPosition.en_passant = selectedSquare - 8;
                } else if(selectedSquare - targetedSquare == -16){
                        // if the pawn moves two squares forward, set the en passant square
                        newPosition.en_passant = selectedSquare + 8;
                } else {
                        // otherwise, reset the en passant square
                        newPosition.en_passant = -1;
                }
                if((targetedSquare / 8 == 0 ) || (targetedSquare / 8 == 7 )){
                        // if the pawn reaches the last rank, promote it
                        // TODO: add some visual way to choose our piece
                        // and also a way for the engin to choos a good piece
                        newPosition.pieces[currentPosition.side_to_move][PAWN] &= ~(1ULL << targetedSquare); // remove the pawn from the target square
                        newPosition.pieces[currentPosition.side_to_move][promotionPiece] |= (1ULL << targetedSquare); // place the promoted piece on the target square               
                }
                // set the clock for 50 move rule to 0
                newPosition.halfmove_clock = 0;
        }

        //handle all the castling
        if(selectedPieceType == KING && currentPosition.castling_rights > 0){
                if(selectedSquare - targetedSquare == -2){// kingside chastle
                        if(currentPosition.side_to_move == COLOR_WHITE){
                                newPosition.pieces[COLOR_WHITE][ROOK] &= ~(1ULL << 63); // delet the old rook
                                newPosition.pieces[COLOR_WHITE][ROOK] |= 1ULL << 61;    // move the rook
                                newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << 63);     // remove the old occupancy
                                newPosition.occupancy[COLOR_BOTH] |= 1ULL << 61;        // add the new occupancy
                                newPosition.occupancy[COLOR_WHITE] &= ~(1ULL << 63);     // remove the old occupancy
                                newPosition.occupancy[COLOR_WHITE] |= 1ULL << 61;        // add the new occupancy
                                newPosition.castling_rights &= ~(3ULL << 2);
                        }
                        if(currentPosition.side_to_move == COLOR_BLACK){
                                newPosition.pieces[COLOR_BLACK][ROOK] &= ~(1ULL << 7); // delet the old rook
                                newPosition.pieces[COLOR_BLACK][ROOK] |= 1ULL << 5;    // move the rook
                                newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << 7);     // remove the old occupancy
                                newPosition.occupancy[COLOR_BOTH] |= 1ULL << 5;        // add the new occupancy
                                newPosition.occupancy[COLOR_BLACK] &= ~(1ULL << 7);     // remove the old occupancy
                                newPosition.occupancy[COLOR_BLACK] |= 1ULL << 5;        // add the new occupancy
                                newPosition.castling_rights &= ~(3ULL << 0);
                        }
                }else if(selectedSquare - targetedSquare == 2){// queenside chastle
                        if(currentPosition.side_to_move == COLOR_WHITE){
                                newPosition.pieces[COLOR_WHITE][ROOK] &= ~(1ULL << 56); // delet the old rook
                                newPosition.pieces[COLOR_WHITE][ROOK] |= 1ULL << 59;    // move the rook
                                newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << 56);     // remove the old occupancy
                                newPosition.occupancy[COLOR_BOTH] |= 1ULL << 59;        // add the new occupancy
                                newPosition.occupancy[COLOR_WHITE] &= ~(1ULL << 56);     // remove the old occupancy
                                newPosition.occupancy[COLOR_WHITE] |= 1ULL << 59;        // add the new occupancy
                                newPosition.castling_rights &= ~(3ULL << 2);
                        }
                        if(currentPosition.side_to_move == COLOR_BLACK){
                                newPosition.pieces[COLOR_BLACK][ROOK] &= ~(1ULL << 0); // delet the old rook
                                newPosition.pieces[COLOR_BLACK][ROOK] |= 1ULL << 3;    // move the rook
                                newPosition.occupancy[COLOR_BOTH] &= ~(1ULL << 0);     // remove the old occupancy
                                newPosition.occupancy[COLOR_BOTH] |= 1ULL << 3;        // add the new occupancy
                                newPosition.occupancy[COLOR_BLACK] &= ~(1ULL << 0);     // remove the old occupancy
                                newPosition.occupancy[COLOR_BLACK] |= 1ULL << 3;        // add the new occupancy
                                newPosition.castling_rights &= ~(3ULL << 0);
                        }
                }else{
                        // if the king move but didn't castle, remove its castling rights
                        if(currentPosition.side_to_move == COLOR_WHITE){
                                newPosition.castling_rights &= ~(3ULL << 2);
                        }else{
                                newPosition.castling_rights &= ~(3ULL << 0);
                        }
                }
        }

        // delet the castling right if a rook moves
        if(selectedPieceType == ROOK && currentPosition.castling_rights > 0){
                if(selectedSquare == 0){newPosition.castling_rights &= ~(1ULL << 0);}
                if(selectedSquare == 7){newPosition.castling_rights &= ~(1ULL << 1);}
                if(selectedSquare == 56){newPosition.castling_rights &= ~(1ULL << 2);}
                if(selectedSquare == 63){newPosition.castling_rights &= ~(1ULL << 3);}
        }
        // delet the castling right if a rook get captured
        if(isTargetedSquareOccupied == 0 && currentPosition.castling_rights > 0){
                if(targetedSquare == 0){newPosition.castling_rights &= ~(1ULL << 0);}
                if(targetedSquare == 7){newPosition.castling_rights &= ~(1ULL << 1);}
                if(targetedSquare == 56){newPosition.castling_rights &= ~(1ULL << 2);}
                if(targetedSquare == 63){newPosition.castling_rights &= ~(1ULL << 3);}
        }

        
        if(newPosition.side_to_move == COLOR_BLACK){
                newPosition.fullmove_number++;
                newPosition.halfmove_clock++;
        }

        // switch the active player side
        newPosition.side_to_move = 1 - newPosition.side_to_move;
        return newPosition;
}

// Find the piece and color off a piece on any square of a board and returns them in the pieceColor and pieceType pointer
int findPieceOnPosition(int square, Position position, int* pieceColor, int* pieceType){
        for(int piece = 0; piece < 6; piece++){
                for(int color = 0; color < 2; color++){
                        if(position.pieces[color][piece] >> square & 1ULL){
                                // we found the piece on the square
                                if (pieceColor != NULL) *pieceColor = color;
                                if (pieceType != NULL) *pieceType = piece;
                                return 0; // square is occupied
                        }
                }
        }
        return -1; // no piece found on the square
}

bitboard legalMovesPawn(Position position, int square, int color){
        bitboard moves = 0;

        int opponent = 1 - color;
        int file = square % 8;
        int rank = square / 8;

        if(color == COLOR_WHITE){
                // move forward
                if(!(rank < 7 && (position.occupancy[COLOR_BOTH] >> (square - 8) & 1ULL))){
                        moves |= (1ULL << (square - 8));
                        // double move from starting position
                        if(rank == 6 && !(position.occupancy[COLOR_BOTH] >> (square - 16) & 1ULL)){
                                moves |= (1ULL << (square - 16));
                        }
                }
                // captures
                if(rank < 7 && file < 7 && (position.occupancy[opponent] >> (square - 7) & 1ULL)){
                        moves |= (1ULL << (square - 7));
                }
                if(rank < 7 && file > 0 && (position.occupancy[opponent] >> (square - 9) & 1ULL)){
                        moves |= (1ULL << (square - 9));
                }       
                // en passant (white pawns on rank 4, capture on rank 3)
                if(rank == 3 && position.en_passant > -1){ 
                        if(position.en_passant == square - 9){ // capture left
                                moves |= (1ULL << (square - 9));
                        }
                        if(file < 7 && position.en_passant == square - 7){ // capture right
                                moves |= (1ULL << (square - 7));
                        }
                }
        } else {
                // move forward
                if(rank > 0 && !(position.occupancy[COLOR_BOTH] >> (square + 8) & 1ULL)){
                        moves |= (1ULL << (square + 8));
                        // double move from starting position
                        if(rank == 1 && !(position.occupancy[COLOR_BOTH] >> (square + 16) & 1ULL)){
                                moves |= (1ULL << (square + 16));
                        }
                }
                // captures
                if(rank > 0 && file < 7 && (position.occupancy[opponent] >> (square + 9) & 1ULL)){
                        moves |= (1ULL << (square + 9));
                }
                if(rank > 0 && file > 0 && (position.occupancy[opponent] >> (square + 7) & 1ULL)){
                        moves |= (1ULL << (square + 7));
                }
                // en passant (black pawns on rank 3, capture on rank 4)
                if(rank == 4 && position.en_passant > -1){ 
                        if(position.en_passant == square + 7){ // capture left
                                moves |= (1ULL << (square + 7));
                        }
                        if(position.en_passant == square + 9){ // capture right
                                moves |= (1ULL << (square + 9));
                        }
                }
        }
        return moves;
}

bitboard legalMovesRook(Position position, int square){
        bitboard moves = 0;
        for(int i = 1; i < 8; i++){
                if(square + i * 8 < 64){ // move up
                        if(position.occupancy[position.side_to_move] >> (square + i * 8) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square + i * 8));
                        if(position.occupancy[COLOR_BOTH] >> (square + i * 8) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }
        for(int i = 1; i < 8; i++){
                if(square - i * 8 >= 0){ // move down
                        if(position.occupancy[position.side_to_move] >> (square - i * 8) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square - i * 8));
                        if(position.occupancy[COLOR_BOTH] >> (square - i * 8) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }
        for(int i = 1; i < 8; i++){
                if((square % 8) + i < 8){ // move right
                        if(position.occupancy[position.side_to_move] >> (square + i) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square + i));
                        if(position.occupancy[COLOR_BOTH] >> (square + i) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }                
        for(int i = 1; i < 8; i++){
                if((square % 8) - i >= 0){ // move left
                        if(position.occupancy[position.side_to_move] >> (square - i) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square - i));
                        if(position.occupancy[COLOR_BOTH] >> (square - i) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }
        return moves;
}

bitboard legalMovesKnight(Position position, int square){
        bitboard moves = 0;
        int file = square % 8;
        int rank = square / 8;
        const int knightMoves[8] = {17, 15, 10, 6, -17, -15, -10, -6};
        for(int i = 0; i < 8; i++){
                int target = square + knightMoves[i];
                if(target < 0 || target >= 64) continue;

                int targetFile = target % 8;
                int targetRank = target / 8;
                int fileDiff = targetFile - file;
                int rankDiff = targetRank - rank;

                if((abs(fileDiff) == 1 && abs(rankDiff) == 2) || (abs(fileDiff) == 2 && abs(rankDiff) == 1)){
                        if(!(position.occupancy[position.side_to_move] >> target & 1ULL)){
                                moves |= (1ULL << target);
                        }
                }
        }
        return moves;
}

bitboard legalMovesBishop(Position position, int square){
        bitboard moves = 0;
        for(int i = 1; i < 8; i++){
                if((square % 8) + i < 8 && (square / 8) + i < 8){ // move up-right
                        if(position.occupancy[position.side_to_move] >> (square + i * 9) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square + i * 9));
                        if(position.occupancy[COLOR_BOTH] >> (square + i * 9) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }
        for(int i = 1; i < 8; i++){
                if((square % 8) - i >= 0 && (square / 8) + i < 8){ // move up-left
                        if(position.occupancy[position.side_to_move] >> (square + i * 7) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square + i * 7));
                        if(position.occupancy[COLOR_BOTH] >> (square + i * 7) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }
        for(int i = 1; i < 8; i++){
                if((square % 8) + i < 8 && (square / 8) - i >= 0){ // move down-right
                        if(position.occupancy[position.side_to_move] >> (square - i * 7) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square - i * 7));
                        if(position.occupancy[COLOR_BOTH] >> (square - i * 7) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }                
        for(int i = 1; i < 8; i++){
                if((square % 8) - i >= 0 && (square / 8) - i >= 0){ // move down-left
                        if(position.occupancy[position.side_to_move] >> (square - i * 9) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                        moves |= (1ULL << (square - i * 9));
                        if(position.occupancy[COLOR_BOTH] >> (square - i * 9) & 1ULL){ // if there is a piece in the way, stop looking in that direction
                                break;
                        }
                }
        }
        return moves;
}

bitboard legalMovesQueen(Position position, int square){
        bitboard moves = legalMovesRook(position, square) | legalMovesBishop(position, square);
        return moves;
}

bitboard legalMovesKing(Position position, int square){
        bitboard moves = pseudoLegalMovesKing(position, square);
        for(int i = 0; i < 64; i++){
                if(!(moves & (1ULL << i))){continue;}// square is not even reachable in the first place
                if(isSquareAttacked(position, i, 1 - position.side_to_move) == 1){
                        moves &= ~(1ULL << i);
                }
        }
        // lock for possable chastling
        if(square == 60 && position.side_to_move == COLOR_WHITE){
                //White Kingside Chastle
                if(position.castling_rights & (1ULL << 3)){
                        // check if the squares are empty
                        if(!(position.occupancy[COLOR_BOTH] & 0x6000000000000000)){ //this hexcode represents the 2 squars that need to be free for castling
                                // check if the casteling is legal
                                if(isSquareAttacked(position, 60, COLOR_BLACK) == 0 &&
                                   isSquareAttacked(position, 61, COLOR_BLACK) == 0 &&
                                   isSquareAttacked(position, 62, COLOR_BLACK) == 0){
                                        // safe to castle
                                        moves |= (1ULL << 62);  
                                } 
                        }
                         
                }
                //White Queensideside Chastle
                if(position.castling_rights & (1ULL << 2)){
                        // check if the squares are empty
                        if(!(position.occupancy[COLOR_BOTH] & 0x0E00000000000000)){ //this hexcode represents the 2 squars that need to be free for castling
                                // check if the casteling is legal
                                if(isSquareAttacked(position, 60, COLOR_BLACK) == 0 &&
                                   isSquareAttacked(position, 59, COLOR_BLACK) == 0 &&
                                   isSquareAttacked(position, 58, COLOR_BLACK) == 0){
                                        // safe to castle
                                        moves |= (1ULL << 58);  
                                } 
                        }
                         
                }
        }
        if(square == 4 && position.side_to_move == COLOR_BLACK){
                //Black Kingside Chastle
                if(position.castling_rights & (1ULL << 1)){
                        // check if the squares are empty
                        if(!(position.occupancy[COLOR_BOTH] & 0x0000000000000060)){ //this hexcode represents the 2 squars that need to be free for castling
                                // check if the casteling is legal
                                if(isSquareAttacked(position, 4, COLOR_WHITE) == 0 &&
                                   isSquareAttacked(position, 5, COLOR_WHITE) == 0 &&
                                   isSquareAttacked(position, 6, COLOR_WHITE) == 0){
                                        // safe to castle
                                        moves |= (1ULL << 6);  
                                } 
                        }
                         
                }
                //Black Queensideside Chastle
                if(position.castling_rights & (1ULL << 0)){
                        // check if the squares are empty
                        if(!(position.occupancy[COLOR_BOTH] & 0x000000000000000E)){ //this hexcode represents the 2 squars that need to be free for castling
                                // check if the casteling is legal
                                if(isSquareAttacked(position, 4, COLOR_WHITE) == 0 &&
                                   isSquareAttacked(position, 3, COLOR_WHITE) == 0 &&
                                   isSquareAttacked(position, 2, COLOR_WHITE) == 0){
                                        // safe to castle
                                        moves |= (1ULL << 2);  
                                } 
                        }
                         
                }
        }

        return moves;
}

// returns all of the legal kingmoves, regarding any checks and that stuff
bitboard pseudoLegalMovesKing(Position position, int square) {
        bitboard moves = 0ULL;
        const int kingMovement[8] = {1, -1, 8, -8, 7, -7, 9, -9};

        int file = square % 8;
        int rank = square / 8;

        for(int i = 0; i < 8; i++) {
                int targetedSquare = square + kingMovement[i];

                // ensure the square is actually on the 8x8 board
                if(targetedSquare < 0 || targetedSquare > 63) continue;

                // prevent wrapping around edges
                // if the king moves more than 1 file/rank distance, it's a wrap
                int targetFile = targetedSquare % 8;
                int targetRank = targetedSquare / 8;

                if (abs(file - targetFile) > 1 || abs(rank - targetRank) > 1) continue;

                // check if the square is occupied by on of our own pieces
                if (!(position.occupancy[position.side_to_move] & (1ULL << targetedSquare))) {
                moves |= (1ULL << targetedSquare);
                }
        }
        return moves;
}


// return a 0 if a move is legal, and a 1 if it is not
int isMoveLegal(Position currentPosition, int startSquare, int targetSquare){
        // Simulate the move on a temporary board copy
        Position testPosition = makeMove(startSquare, targetSquare, currentPosition, QUEEN); // what you promot to is irrelevent in terms of legalness in a position

        // Because makeMove already flipped the side_to_move, the moving side is now the opponent
        int movingColor = 1 - testPosition.side_to_move; 
        int enemyColor = testPosition.side_to_move;

        int kingSquare = getKingSquare(testPosition, movingColor);

        // 3. If the king is under attack after making the move, it's illegal
        if(isSquareAttacked(testPosition, kingSquare, enemyColor)) {
                return 1; // Illegal move (left king in check, walked into check, or failed to block)
        }

        return 0; // Completely legal move
}

int getKingSquare(Position position, int color){
        bitboard kingBitboard = position.pieces[color][KING];
        if (kingBitboard == 0) return -1; // Should never happen in a real game
        
        // Efficient way to find the index of the only bit set to 1
        #if defined(__GNUC__) || defined(__clang__)
                return __builtin_ctzll(kingBitboard);
        #else
                // Fallback loop if not using GCC/Clang
                for (int i = 0; i < 64; i++) {
                if ((kingBitboard >> i) & 1ULL) return i;
                }
                return -1;
        #endif
}

// returns a 0 if a square is not attacked and a 1 if it is
int isSquareAttacked(Position position, int square, int attackerColor){
        int file = square % 8;
        int rank = square / 8;

        // 1. Pawn Attacks
        // We check backwards from the perspective of the attacker.
        if(attackerColor == COLOR_WHITE){
        // White pawns move towards lower indices (based on the legalMovesPawn logic)
        // Therefore, if White is attacking this square, their pawns must be at +7 or +9
                if (file > 0 && square + 7 < 64 && (position.pieces[COLOR_WHITE][PAWN] & (1ULL << (square + 7)))) return 1;
                if (file < 7 && square + 9 < 64 && (position.pieces[COLOR_WHITE][PAWN] & (1ULL << (square + 9)))) return 1;
        }else{
                // Black pawns move towards higher indices
                // Therefore, if Black is attacking this square, their pawns must be at -7 or -9
                if (file < 7 && square - 7 >= 0 && (position.pieces[COLOR_BLACK][PAWN] & (1ULL << (square - 7)))) return 1;
                if (file > 0 && square - 9 >= 0 && (position.pieces[COLOR_BLACK][PAWN] & (1ULL << (square - 9)))) return 1;
        }

        // 2. Knight Attacks
        const int knightMoves[8] = {17, 15, 10, 6, -17, -15, -10, -6};
        for (int i = 0; i < 8; i++){
                int target = square + knightMoves[i];
                if(target >= 0 && target < 64) {
                int targetFile = target % 8;
                int targetRank = target / 8;
                // Prevent board wrapping
                if((abs(targetFile - file) == 1 && abs(targetRank - rank) == 2) || 
                        (abs(targetFile - file) == 2 && abs(targetRank - rank) == 1)) {
                        if(position.pieces[attackerColor][KNIGHT] & (1ULL << target)) return 1;
                }
                }
        }

        // 3. King Attacks
        // (not really attacks, but just squares it blocks off)
        const int kingMovement[8] = {1, -1, 8, -8, 7, -7, 9, -9};
        for(int i = 0; i < 8; i++) {
                int target = square + kingMovement[i];
                if(target >= 0 && target < 64) {
                if(abs((target % 8) - file) <= 1 && abs((target / 8) - rank) <= 1) {
                        if(position.pieces[attackerColor][KING] & (1ULL << target)) return 1;
                }
                }
        }

        // 4. Straight-line sliders (Rook & Queen)
        // Up (+8)
        for (int i = 1; i < 8; i++) {
                int target = square + i * 8;
                if(target >= 64) break;
                if((position.pieces[attackerColor][ROOK] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break; // Blocked by any piece
        }
        // Down (-8)
        for(int i = 1; i < 8; i++) {
                int target = square - i * 8;
                if(target < 0) break;
                if((position.pieces[attackerColor][ROOK] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break; // Blocked by any piece
        }
        // Right (+1)
        for (int i = 1; i < 8; i++) {
                if((square % 8) + i >= 8) break; // Board edge
                int target = square + i;
                if((position.pieces[attackerColor][ROOK] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break; // Blocked by any piece
        }
        // Left (-1)
        for(int i = 1; i < 8; i++) {
                if((square % 8) - i < 0) break; // Board edge
                int target = square - i;
                if((position.pieces[attackerColor][ROOK] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break; // Blocked by any piece
        }

        // 5. Diagonal sliders (Bishop & Queen)
        // Up-Right (+9)
        for(int i = 1; i < 8; i++) {
                if((square % 8) + i >= 8 || (square / 8) + i >= 8) break;
                int target = square + i * 9;
                if((position.pieces[attackerColor][BISHOP] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break;
        }
        // Up-Left (+7)
        for (int i = 1; i < 8; i++) {
                if((square % 8) - i < 0 || (square / 8) + i >= 8) break;
                int target = square + i * 7;
                if((position.pieces[attackerColor][BISHOP] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break;
        }
        // Down-Right (-7)
        for (int i = 1; i < 8; i++) {
                if((square % 8) + i >= 8 || (square / 8) - i < 0) break;
                int target = square - i * 7;
                if((position.pieces[attackerColor][BISHOP] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break;
        }
        // Down-Left (-9)
        for (int i = 1; i < 8; i++) {
                if((square % 8) - i < 0 || (square / 8) - i < 0) break;
                int target = square - i * 9;
                if((position.pieces[attackerColor][BISHOP] | position.pieces[attackerColor][QUEEN]) & (1ULL << target)) return 1;
                if(position.occupancy[COLOR_BOTH] & (1ULL << target)) break;
        }

        // If no attack is found, return 0 (false)
        return 0;
}

// returns the state of any position
int gameState(Position position){
        int side = position.side_to_move;
        int enemy = 1 - side;
        
        // Check if the current king is under attack
        int kingSquare = getKingSquare(position, side);
        int inCheck = isSquareAttacked(position, kingSquare, enemy);
        
        int hasLegalMoves = 0;
        bitboard myPieces = position.occupancy[side];

        // Scan through ONLY the squares that have our pieces on them
        while (myPieces) {
                // Find the index of the next piece
                #if defined(__GNUC__) || defined(__clang__)
                int square = __builtin_ctzll(myPieces);
                #else
                int square = -1;
                for (int i = 0; i < 64; i++) {
                        if ((myPieces >> i) & 1ULL) { square = i; break; }
                }
                #endif

                // If this piece has even ONE legal move, the game is not over
                if (legalMoves(position, square) != 0ULL) {
                        hasLegalMoves = 1;
                        break; 
                }

                // Clear the bit we just checked so we can evaluate the next piece
                myPieces &= (myPieces - 1); 
        }

        // Evaluate the results
        if (!hasLegalMoves) {
                if (inCheck) {
                        return STATE_CHECKMATE; // No moves + In Check = Lose
                } else {
                        return STATE_STALEMATE; // No moves + Not in Check = Draw
                }
        }

        // If they have moves but the king is targeted, it's just a regular check
        if (inCheck) {
                return STATE_CHECK;
        }

        return STATE_ONGOING;
}