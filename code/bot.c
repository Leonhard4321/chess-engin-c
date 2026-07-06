#include "main.h"
#include <limits.h>

static int negamax(Position position, int depth, int alpha, int beta);
static int evaluateRelative(Position position, int rootColor);
static int evaluateTerminal(Position position, int rootColor, int gameState);
static int getMovePromotion(Position position, int selectedSquare, int targetedSquare);
static void findBestMove(Position position, int depth, int rootColor, int *bestFrom, int *bestTo, int *bestPromotion);
static int getSearchDepth(Position position);
// forward eval prototypes (used by quiescence)
int evaluatePosition(Position position);
int evaluatePiece(bitboard pieces, int pointsPerPiece);

// Move ordering / lists
typedef struct { int from; int to; int promotion; int score; } Move;
static int pieceValue[6] = {100,500,320,330,900,20000};
static void generateMoveList(Position position, Move *out, int *outCount, int onlyCaptures){
        int count = 0;
        bitboard myPieces = position.occupancy[position.side_to_move];
        while(myPieces){
                int from = __builtin_ctzll(myPieces);
                myPieces &= myPieces - 1;
                bitboard moves = legalMoves(position, from);
                while(moves){
                        int to = __builtin_ctzll(moves);
                        moves &= moves - 1;
                        int capturedColor, capturedType;
                        int occ = findPieceOnPosition(to, position, &capturedColor, &capturedType);
                        if(onlyCaptures && occ == -1) continue;
                        Move m = {from, to, getMovePromotion(position, from, to), 0};
                        if(occ == 0){ // capture
                                m.score = pieceValue[capturedType];
                        } else {
                                // small heuristic: prefer center and promotions
                                int file = to % 8;
                                int rank = to / 8;
                                int centerScore = 4 - abs(file - 3) - abs(rank - 3);
                                m.score = centerScore;
                        }
                        out[count++] = m;
                }
        }
        // simple insertion sort by descending score
        for(int i = 1; i < count; i++){
                Move key = out[i];
                int j = i - 1;
                while(j >= 0 && out[j].score < key.score){ out[j+1] = out[j]; j--; }
                out[j+1] = key;
        }
        *outCount = count;
}

// quiescence search: extends captures only
static int quiescence(Position position, int alpha, int beta){
        int stand = evaluatePosition(position); // from side-to-move perspective
        if(stand >= beta) return stand;
        if(alpha < stand) alpha = stand;

        Move moves[256]; int n=0;
        generateMoveList(position, moves, &n, 1);
        for(int i = 0; i < n; i++){
                Move m = moves[i];
                Position next = makeMove(m.from, m.to, position, m.promotion);
                int score = -quiescence(next, -beta, -alpha);
                if(score >= beta) return score;
                if(score > alpha) alpha = score;
        }
        return alpha;
}

int evaluatePosition(Position position);
int evaluatePiece(bitboard pieces, int pointsPerPiece);

// Global position history for threefold detection
static Position *g_positionHistory = NULL;
static int g_historyCount = 0;

static int isPositionInHistory(Position position){
        if(!g_positionHistory) return 0;
        int count = 0;
        for(int i = 0; i < g_historyCount; i++){
                if(g_positionHistory[i].pieces[0][0] == position.pieces[0][0] &&
                   g_positionHistory[i].pieces[0][1] == position.pieces[0][1] &&
                   g_positionHistory[i].pieces[0][2] == position.pieces[0][2] &&
                   g_positionHistory[i].pieces[0][3] == position.pieces[0][3] &&
                   g_positionHistory[i].pieces[0][4] == position.pieces[0][4] &&
                   g_positionHistory[i].pieces[0][5] == position.pieces[0][5] &&
                   g_positionHistory[i].pieces[1][0] == position.pieces[1][0] &&
                   g_positionHistory[i].pieces[1][1] == position.pieces[1][1] &&
                   g_positionHistory[i].pieces[1][2] == position.pieces[1][2] &&
                   g_positionHistory[i].pieces[1][3] == position.pieces[1][3] &&
                   g_positionHistory[i].pieces[1][4] == position.pieces[1][4] &&
                   g_positionHistory[i].pieces[1][5] == position.pieces[1][5] &&
                   g_positionHistory[i].side_to_move == position.side_to_move){
                        count++;
                }
        }
        return count >= 2; // Avoid if position appears twice (3rd time would be draw)
}

int handleBot(Position *position, int *gameOver, Position *positionHistory, int historyCount){
        int bestFrom = -1;
        int bestTo = -1;
        int bestPromotion = QUEEN;
        int rootColor = position->side_to_move;
        
        // Set global history for evaluation
        g_positionHistory = positionHistory;
        g_historyCount = historyCount;

        findBestMove(*position, getSearchDepth(*position), rootColor, &bestFrom, &bestTo, &bestPromotion);
        int madeMove = 0;
        if(bestFrom >= 0){
                *position = makeMove(bestFrom, bestTo, *position, bestPromotion);
                *gameOver = gameState(*position);
                madeMove = 1;
        }
        
        // Clear history pointer
        g_positionHistory = NULL;
        g_historyCount = 0;
        return madeMove;
}

// returns a number that determents which player is better
// a high number means the currently moving player is better
// a low number means the currently moving player is worse
// a zero means it is equal
int evaluatePosition(Position position){
        int colorBot = position.side_to_move;
        int colorPlayer = 1 - colorBot;

        int evaluationBot = 0;
        evaluationBot += evaluatePiece(position.pieces[colorBot][PAWN], 100);
        evaluationBot += evaluatePiece(position.pieces[colorBot][KNIGHT], 320);
        evaluationBot += evaluatePiece(position.pieces[colorBot][BISHOP], 330);
        evaluationBot += evaluatePiece(position.pieces[colorBot][ROOK], 500);
        evaluationBot += evaluatePiece(position.pieces[colorBot][QUEEN], 900);
        evaluationBot += evaluatePiece(position.pieces[colorBot][KING], 20000);

        int evaluationPlayer = 0;
        evaluationPlayer += evaluatePiece(position.pieces[colorPlayer][PAWN], 100);
        evaluationPlayer += evaluatePiece(position.pieces[colorPlayer][KNIGHT], 320);
        evaluationPlayer += evaluatePiece(position.pieces[colorPlayer][BISHOP], 330);
        evaluationPlayer += evaluatePiece(position.pieces[colorPlayer][ROOK], 500);
        evaluationPlayer += evaluatePiece(position.pieces[colorPlayer][QUEEN], 900);
        evaluationPlayer += evaluatePiece(position.pieces[colorPlayer][KING], 20000);
        
        int eval = evaluationBot - evaluationPlayer;
        
        // Penalize positions that lead to threefold repetition
        if(isPositionInHistory(position)){
                eval -= 150; // Small penalty to avoid draw
        }

        return eval;
}

int evaluatePiece(bitboard pieces, int pointsPerPiece){
        int total = 0;
        for(int square = 0; square < 64; square++){
                if(pieces & (1ULL << square)){
                        total += pointsPerPiece;
                }
        }
        return total;
}

static int evaluateRelative(Position position, int rootColor){
        int score = evaluatePosition(position);
        return position.side_to_move == rootColor ? score : -score;
}

static int evaluateTerminal(Position position, int rootColor, int state){
        if(state == STATE_CHECKMATE){
                return position.side_to_move == rootColor ? -1000000 : 1000000;
        }
        return 0;
}

static int getMovePromotion(Position position, int selectedSquare, int targetedSquare){
        int pieceType;
        findPieceOnPosition(selectedSquare, position, NULL, &pieceType);
        if(pieceType != PAWN){
                return QUEEN;
        }

        int targetRank = targetedSquare / 8;
        if((position.side_to_move == COLOR_WHITE && targetRank == 0) ||
           (position.side_to_move == COLOR_BLACK && targetRank == 7)){
                return QUEEN;
        }

        return QUEEN;
}

static int getSearchDepth(Position position){
        int pieceCount = 0;
        for(int color = 0; color < 2; color++){
                for(int pieceType = 0; pieceType < 6; pieceType++){
                        pieceCount += __builtin_popcountll(position.pieces[color][pieceType]);
                }
        }

        Move moves[512];
        int moveCount = 0;
        generateMoveList(position, moves, &moveCount, 0);

        int complexity = pieceCount + (moveCount / 3);
        if(complexity <= 12) return 7;
        if(complexity <= 18) return 6;
        if(complexity <= 24) return 5;
        return 4;
}

static int negamax(Position position, int depth, int alpha, int beta){
        int state = gameState(position);
        if(state == STATE_CHECKMATE){
                return -1000000; // losing for side to move
        }
        if(state == STATE_STALEMATE){
                return 0;
        }
        if(depth == 0){
                return quiescence(position, alpha, beta);
        }

        Move moves[512]; int n = 0;
        generateMoveList(position, moves, &n, 0);
        if(n == 0) return 0; // no moves but should be caught by gameState

        int best = INT_MIN;
        for(int i = 0; i < n; i++){
                Move m = moves[i];
                Position nextPos = makeMove(m.from, m.to, position, m.promotion);
                int score = -negamax(nextPos, depth - 1, -beta, -alpha);
                if(score > best) best = score;
                if(score > alpha) alpha = score;
                if(alpha >= beta) break;
        }
        return best;
}

static void findBestMove(Position position, int depth, int rootColor, int *bestFrom, int *bestTo, int *bestPromotion){
        int maximizing = (position.side_to_move == rootColor);
        int bestScore = maximizing ? INT_MIN : INT_MAX;

        // generate ordered moves
        Move moves[512]; int n = 0;
        generateMoveList(position, moves, &n, 0);
        for(int i = 0; i < n; i++){
                Move m = moves[i];
                Position nextPos = makeMove(m.from, m.to, position, m.promotion);
                int score = -negamax(nextPos, depth - 1, INT_MIN, INT_MAX); // score from root perspective
                if((maximizing && score > bestScore) || (!maximizing && score < bestScore)){
                        bestScore = score;
                        *bestFrom = m.from;
                        *bestTo = m.to;
                        *bestPromotion = m.promotion;
                }
        }
}
