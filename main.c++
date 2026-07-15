#include <cstdint>
using namespace std;

uint64_t startingPawns = 0x000000000000FF00 & 0xff00ffffffffffff;

enum PieceTypes
{
    Pawn,
    Knight,
    Bishop,
    Rook,
    King,
    Queen
};

enum Color
{
    White = 0,
    Black = 1
};

class Board
{
public:
    uint64_t pieces[2][6];
    // {x, x, x, x, x, x}
    // {x, x, x, x, x, x}

    uint64_t whitePieces;
    uint64_t blackPieces;
    uint64_t allPieces;

    Color sideToMove;

    int castlingRights;
    int enPassantSquare;

    Board()
    {
        for (int c = 0; c < 2; ++c)
        {
            for (int p = 0; p < 6; ++p)
            {
                pieces[c][p] = 0ULL;
            }
        }

        pieces[White][Pawn] = 0x000000000000FF00;
        pieces[White][Rook] = 0x0000000000000081;
        pieces[White][Knight] = 0x0000000000000042;
        pieces[White][Bishop] = 0x0000000000000024;
        pieces[White][Queen] = 0x0000000000000008;
        pieces[White][King] = 0x0000000000000010;

        pieces[Black][Pawn] = 0x00FF000000000000;
        pieces[Black][Rook] = 0x8100000000000000;
        pieces[Black][Knight] = 0x4200000000000000;
        pieces[Black][Bishop] = 0x2400000000000000;
        pieces[Black][Queen] = 0x0800000000000000;
        pieces[Black][King] = 0x1000000000000000;

        updateOccupancy();
    }

    void updateOccupancy()
    {
        whitePieces = 0;
        blackPieces = 0;
        for (int i = 0; i < 6; ++i)
        {
            whitePieces |= pieces[White][i];
            blackPieces |= pieces[Black][i];
        }
        allPieces = whitePieces | blackPieces;
    }
};

struct movesList {
    uint16_t moves[256];
    int count = 0;

    void addMove(uint16_t move){
        moves[count] = move;
        ++count;
    }
};

int main()
{
    Board chessBoard;
};

movesList generateMoves(const Board& board) {

    uint16_t move; // 0-5 source, 6-11 destination, 12-13 promotion piece, 13-16 flags (en passsant, castle, double push)

    movesList moves;

    return moves;
}

void serializePawnMoves(const Board& board, movesList& list){

    uint16_t emptySquares = ~board.allPieces;
    
    uint16_t rank4 = 0xff000000;

    uint64_t wPawns = board.pieces[White][Pawn];
    uint64_t singlePushes = (wPawns << 8) & emptySquares;
    while (singlePushes != 0){
        int dest = __builtin_ctzll(singlePushes);
        int src = dest - 8;
        
        uint16_t move = src | (dest << 6);

        list.addMove(move);

        singlePushes &= (singlePushes-1);

    }
    uint64_t doublePushes = (singlePushes << 8) & emptySquares & rank4;

}