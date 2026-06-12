#include "main.h"

bitboard legalMovesPawn(Position position, int square);
bitboard legalMovesRook(Position position, int square);
bitboard legalMovesKnight(Position position, int square);
bitboard legalMovesBishop(Position position, int square);
bitboard legalMovesQueen(Position position, int square);
bitboard legalMovesKing(Position position, int square);

bitboard legalMoves(Position position, int square){
        int piece = findPieceOnPosition(square, position);
        if(piece == -1) return 0; // no piece on the square
        piece = piece % 6; // get the piece type (0-5)

        if(piece == KING){
                return legalMovesKing(position, square);
        }
        if(isKingCheck(position)){
                return 0ULL;
        }

        if(piece == PAWN){
                return legalMovesPawn(position, square);        
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
                return legalMovesRook(position, square) | legalMovesBishop(position, square);
        }

        return 0x0123456789ABCDEF; // this part of the funtkion should never be reached
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

bitboard legalMovesPawn(Position position, int square){
        bitboard moves = 0;
        int pawn = findPieceOnPosition(square, position);
        if(pawn == -1) return 0;

        int color = pawn / 6;
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
                if(rank > 0 && file < 0 && (position.occupancy[opponent] >> (square + 9) & 1ULL)){
                        moves |= (1ULL << (square + 9));
                }
                if(rank > 0 && file > 7 && (position.occupancy[opponent] >> (square + 7) & 1ULL)){
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

bitboard legalMovesKing(Position position, int square){
        bitboard moves = 0ULL;

        int file = square % 8;
        int rank = square / 8;
        const int kingMoves[8] = {8, 9, 1, -7, -8, -9, -1, 7};
        int opponent = 1 - position.side_to_move;
        
        // Find opponent king position
        int oppKingSquare = -1;
        for(int sq = 0; sq < 64; sq++){
                if(position.pieces[opponent][KING] >> sq & 1ULL){
                        oppKingSquare = sq;
                        break;
                }
        }

        //remove the king so it doesn't hide behind its own shadow
        position.pieces[position.side_to_move][KING] &= ~(1ULL << square); 
        position.occupancy[position.side_to_move] &= ~(1ULL << square);
        position.occupancy[COLOR_BOTH] &= ~(1ULL << square);

        for(int i = 0; i < 8; i++){
                // delet any king that would be on the board from an earlier check
                position.pieces[position.side_to_move][KING] &= 0ULL;

                int target = square + kingMoves[i];
                if(target < 0 || target >= 64) continue;

                int targetFile = target % 8;
                int targetRank = target / 8;
                if(abs(targetFile - file) > 1 || abs(targetRank - rank) > 1) continue;

                // Check if target square is occupied by own piece
                if(position.occupancy[position.side_to_move] >> target & 1ULL) continue;
                
                // Check if target square is adjacent to opponent king
                if(oppKingSquare != -1){
                        int oppKingFile = oppKingSquare % 8;
                        int oppKingRank = oppKingSquare / 8;
                        if(abs(targetFile - oppKingFile) <= 1 && abs(targetRank - oppKingRank) <= 1) continue;
                }

                // move the king into the target and look if it would be in check on that target
                position.pieces[position.side_to_move][KING] = 1ULL << target;
                position.occupancy[position.side_to_move] |= (1ULL << target);
                position.occupancy[COLOR_BOTH] |= (1ULL << target);
                position.occupancy[opponent] &= ~(1ULL << target);  // remove captured piece if any
                
                if(isKingCheck(position) == 1){
                        position.occupancy[opponent] |= (1ULL << target);  // restore occupancy
                        continue;
                }
                position.occupancy[opponent] |= (1ULL << target);  // restore occupancy

                // if it passes all checks add it into the possable moves bitmap
                moves |= 1ULL << target;
        }
        return moves;
}

// This function returns 0 if there is no check, 1 if there is a check
int isKingCheck(Position position){
        int kingSquare = -1;
        for(int i = 0; i < 64; i++){
                if(position.pieces[position.side_to_move][KING] >> i & 1ULL){
                        kingSquare = i;
                        break;
                }
        }
        if(kingSquare == -1) return -1; // should never happen, there should always be a king on the board

        // Check if the king is attacked by any opponent piece
        int opponent = 1 - position.side_to_move;
        for(int oppSquare = 0; oppSquare < 64; oppSquare++){
                if(!(position.occupancy[opponent] >> oppSquare & 1ULL)) continue; // No opponent piece here
                
                // Get piece type
                int oppPiece = (findPieceOnPosition(oppSquare, position) % 6);
                if(oppPiece == -1) continue; // Skip empty squares
                
                // Check if this piece attacks the king's square
                if(oppPiece == PAWN){
                        // Pawn attacks (not moves)
                        if(opponent == COLOR_WHITE){
                                if(oppSquare / 8 >= 1){
                                        if(oppSquare % 8 < 7 && oppSquare - 7 == kingSquare) return 1;
                                        if(oppSquare % 8 > 0 && oppSquare - 9 == kingSquare) return 1;
                                }
                        } else {
                                if(oppSquare / 8 < 7){
                                        if(oppSquare % 8 > 0 && oppSquare + 7 == kingSquare) return 1;
                                        if(oppSquare % 8 < 7 && oppSquare + 9 == kingSquare) return 1;
                                }
                        }
                } else {
                        // Temporarily swap side_to_move so move functions work for opponent
                        int savedSide = position.side_to_move;
                        position.side_to_move = opponent;
                        
                        if(oppPiece == KNIGHT){
                                if(legalMovesKnight(position, oppSquare) >> kingSquare & 1ULL) {
                                        position.side_to_move = savedSide;
                                        return 1;
                                }
                        } else if(oppPiece == BISHOP){
                                if(legalMovesBishop(position, oppSquare) >> kingSquare & 1ULL) {
                                        position.side_to_move = savedSide;
                                        return 1;
                                }
                        } else if(oppPiece == ROOK){
                                if(legalMovesRook(position, oppSquare) >> kingSquare & 1ULL) {
                                        position.side_to_move = savedSide;
                                        return 1;
                                }
                        } else if(oppPiece == QUEEN){
                                if((legalMovesRook(position, oppSquare) | legalMovesBishop(position, oppSquare)) >> kingSquare & 1ULL) {
                                        position.side_to_move = savedSide;
                                        return 1;
                                }
                        }
                        
                        position.side_to_move = savedSide;
                }
        }
        return 0; // No check found
}