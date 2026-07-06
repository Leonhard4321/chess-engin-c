#include "main.h"

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

void setupBot(){
        #include <time.h>
        srand(time(NULL));
}