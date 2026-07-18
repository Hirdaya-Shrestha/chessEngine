#include <cstdint>
#include <iostream>
#include <string>
#include <raylib.h>

using namespace std;

// this is a comment to update git :) (count : 1)

struct PieceTexture
{
    Texture2D textures[2][6];
};

enum PieceTypes
{
    Pawn,
    Knight,
    Bishop,
    Rook,
    King,
    Queen
};

enum myColor
{
    White = 0,
    Black = 1
};

PieceTexture loadPieceTextures()
{
    PieceTexture result{};

    result.textures[White][Pawn] = LoadTexture("assets/whitePawn.png");
    result.textures[White][Bishop] = LoadTexture("assets/whiteBishop.png");
    result.textures[White][Rook] = LoadTexture("assets/whiteRook.png");
    result.textures[White][Knight] = LoadTexture("assets/whiteKnight.png");
    result.textures[White][Queen] = LoadTexture("assets/whiteQueen.png");
    result.textures[White][King] = LoadTexture("assets/whiteKing.png");

    result.textures[Black][Pawn] = LoadTexture("assets/blackPawn.png");
    result.textures[Black][Bishop] = LoadTexture("assets/blackBishop.png");
    result.textures[Black][Rook] = LoadTexture("assets/blackRook.png");
    result.textures[Black][Knight] = LoadTexture("assets/blackKnight.png");
    result.textures[Black][Queen] = LoadTexture("assets/blackQueen.png");
    result.textures[Black][King] = LoadTexture("assets/blackKing.png");

    return result;
}

class Board
{
public:
    uint64_t pieces[2][6];
    // {x, x, x, x, x, x}
    // {x, x, x, x, x, x}

    uint64_t whitePieces;
    uint64_t blackPieces;
    uint64_t allPieces;

    myColor sideToMove = White;

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

struct movesList
{
    uint16_t moves[256];
    int count = 0;

    void addMove(uint16_t move)
    {
        moves[count] = move;
        ++count;
    }
};

uint64_t knightAttacks[64];

void knightLookup()
{
    uint64_t notFileA = 0xFEFEFEFEFEFEFEFEULL;
    uint64_t notFileB = 0xFCFCFCFCFCFCFCFCULL;
    uint64_t notFileG = 0x3F3F3F3F3F3F3F3FULL;
    uint64_t notFileH = 0x7F7F7F7F7F7F7F7FULL;

    for (int square = 0; square < 64; ++square)
    {
        uint64_t knight = 1ULL << square;
        uint64_t attacks = 0ULL;

        // up 2 right 1
        attacks |= (knight << 17) & notFileA;
        // up 2 left 1
        attacks |= (knight << 15) & notFileH;
        // up 1 right 2
        attacks |= (knight << 10) & notFileA & notFileB;
        // up 1 left 2
        attacks |= (knight << 6) & notFileG & notFileH;
        // down 2 right 1
        if (square >= 17)
            attacks |= (knight >> 17) & notFileH;
        // down 2 left 1
        if (square >= 15)
            attacks |= (knight >> 15) & notFileA;
        // down 1 right 2
        if (square >= 10)
            attacks |= (knight >> 10) & notFileH & notFileG;
        // down 1 left 2
        if (square >= 6)
            attacks |= (knight >> 6) & notFileA & notFileB;

        knightAttacks[square] = attacks;
    }
}

uint64_t kingAttacks[64];

void kingLookup()
{
    uint64_t notFileA = 0xFEFEFEFEFEFEFEFEULL;
    uint64_t notFileH = 0x7F7F7F7F7F7F7F7FULL;

    for (int square = 0; square < 64; ++square)
    {
        uint64_t king = 1ULL << square;
        uint64_t attacks = 0ULL;

        // left
        attacks |= (king >> 1) & notFileH;
        // right
        attacks |= (king << 1) & notFileA;
        // up
        attacks |= (king << 8);
        // down
        attacks |= (king >> 8);
        // up-left
        attacks |= (king << 7) & notFileA;
        // up-right
        attacks |= (king << 9) & notFileH;
        // down-right
        attacks |= (king >> 7) & notFileA;
        // down-left
        attacks |= (king >> 9) & notFileH;

        kingAttacks[square] = attacks;
    }
}

void serializePawnMoves(const Board &board, movesList &list)
{

    uint64_t emptySquares = ~board.allPieces;

    if (board.sideToMove == White)
    {
        uint64_t rank4 = 0xff000000ULL;

        uint64_t wPawns = board.pieces[White][Pawn];

        uint64_t singlePushes = (wPawns << 8) & emptySquares;
        uint64_t doublePushes = (singlePushes << 8) & emptySquares & rank4;

        // Single Pushes
        while (singlePushes != 0)
        {
            int dest = __builtin_ctzll(singlePushes);
            int src = dest - 8;

            uint16_t move = src | (dest << 6);

            list.addMove(move);

            singlePushes &= (singlePushes - 1);
        }

        // Double Pushes
        while (doublePushes != 0)
        {
            int dest = __builtin_ctzll(doublePushes);
            int src = dest - 16;

            uint16_t flag = (1 << 14);
            uint16_t move = src | (dest << 6) | flag;

            list.addMove(move);

            doublePushes &= (doublePushes - 1);
        }

        // Captures
        uint64_t notFileH = 0x7F7F7F7F7F7F7F7F;
        uint64_t capturesLeft = (wPawns << 7) & board.blackPieces & notFileH;
        while (capturesLeft != 0)
        {
            int dest = __builtin_ctzll(capturesLeft);
            int src = dest - 7;

            uint16_t move = src | (dest << 6);

            list.addMove(move);

            capturesLeft &= (capturesLeft - 1);
        }

        uint64_t notFileA = 0xFEFEFEFEFEFEFEFE;
        uint64_t capturesRight = (wPawns << 9) & board.blackPieces & notFileA;
        while (capturesRight != 0)
        {
            int dest = __builtin_ctzll(capturesRight);
            int src = dest - 9;

            uint16_t move = src | (dest << 6);

            list.addMove(move);

            capturesRight &= (capturesRight - 1);
        }
    }
    else if (board.sideToMove == Black)
    {
        uint64_t rank5 = 0xff00000000ULL;
        uint64_t bPawns = board.pieces[Black][Pawn];

        uint64_t singlePushes = (bPawns >> 8) & emptySquares;
        uint64_t doublePushes = (singlePushes >> 8) & emptySquares & rank5;

        // Single Pushes
        while (singlePushes != 0)
        {
            int dest = __builtin_ctzll(singlePushes);
            int src = dest + 8;

            uint16_t move = src | (dest << 6);

            list.addMove(move);

            singlePushes &= (singlePushes - 1);
        }

        // Double Pushes
        while (doublePushes != 0)
        {
            int dest = __builtin_ctzll(doublePushes);
            int src = dest + 16;

            uint16_t flag = (1 << 14);
            uint16_t move = src | (dest << 6) | flag;

            list.addMove(move);

            doublePushes &= (doublePushes - 1);
        }

        uint64_t notFileH = 0x7F7F7F7F7F7F7F7F;
        uint64_t capturesLeft = (bPawns >> 9) & board.whitePieces & notFileH;
        while (capturesLeft != 0)
        {
            int dest = __builtin_ctzll(capturesLeft);
            int src = dest + 9;

            uint16_t move = src | (dest << 6);

            list.addMove(move);

            capturesLeft &= (capturesLeft - 1);
        }

        uint64_t notFileA = 0xFEFEFEFEFEFEFEFE;
        uint64_t capturesRight = (bPawns >> 7) & board.whitePieces & notFileA;
        while (capturesRight != 0)
        {
            int dest = __builtin_ctzll(capturesRight);
            int src = dest + 7;

            uint16_t move = src | (dest << 6);

            list.addMove(move);

            capturesRight &= (capturesRight - 1);
        }
    }
}

void serializeKnightMoves(const Board &board, movesList &list)
{
    myColor play = board.sideToMove;
    uint64_t myKnights = board.pieces[play][Knight];
    uint64_t playPieces = (play == White) ? board.whitePieces : board.blackPieces;

    while (myKnights != 0)
    {
        int src = __builtin_ctzll(myKnights);

        uint64_t validMoves = knightAttacks[src] & ~playPieces;

        while (validMoves != 0)
        {
            int dest = __builtin_ctzll(validMoves);

            uint16_t move = src | dest << 6;

            list.addMove(move);

            validMoves &= (validMoves - 1);
        }
        myKnights &= (myKnights - 1);
    }
}

void serializeKingMoves(const Board &board, movesList &list)
{
    myColor play = board.sideToMove;
    uint64_t piece = board.pieces[play][King];
    uint64_t ownPiece = play == White ? board.whitePieces : board.blackPieces;

    int src = __builtin_ctzll(piece);
    uint64_t moves = kingAttacks[src] & ~ownPiece;
    while (moves != 0)
    {
        int dest = __builtin_ctzll(moves);

        uint16_t move = src | dest << 6;

        list.addMove(move);
        moves &= (moves - 1);
    }
}

uint64_t slidingMoves(int square, const Board &board, const int directions[][2], int directionsCount)
{
    uint64_t attacks = 0ULL;

    int startRank = square / 8;
    int startFile = square % 8;

    for (int i = 0; i < directionsCount; ++i)
    {
        int rank = startRank + directions[i][0];
        int file = startFile + directions[i][1];

        while (rank >= 0 && rank < 8 && file >= 0 && file < 8)
        {
            int target = rank * 8 + file;
            uint64_t targetMask = 1ULL << target;

            attacks |= targetMask;

            if (board.allPieces & targetMask)
            {
                break;
            }

            rank += directions[i][0];
            file += directions[i][1];
        }
    }

    return attacks;
}

void serializeRookMoves(const Board &board, movesList &list)
{
    const int rookDirections[4][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}};

    myColor side = board.sideToMove;
    uint64_t squares = board.pieces[side][Rook];
    uint64_t sidePieces = side == White ? board.whitePieces : board.blackPieces;

    while (squares != 0)
    {
        int src = __builtin_ctzll(squares);
        uint64_t validAttacks = slidingMoves(src, board, rookDirections, 4) & ~sidePieces;
        while (validAttacks != 0)
        {
            int dest = __builtin_ctzll(validAttacks);

            uint16_t move = src | dest << 6;
            list.addMove(move);

            validAttacks &= (validAttacks - 1);
        }

        squares &= (squares - 1);
    }
}

void serializeBishopMoves(const Board &board, movesList &list)
{
    const int bishopDirections[4][2] = {
        {1, 1},
        {-1, 1},
        {1, -1},
        {-1, -1}};

    myColor side = board.sideToMove;
    uint64_t squares = board.pieces[side][Bishop];
    uint64_t sidePieces = side == White ? board.whitePieces : board.blackPieces;

    while (squares != 0)
    {
        int src = __builtin_ctzll(squares);
        uint64_t validAttacks = slidingMoves(src, board, bishopDirections, 4) & ~sidePieces;
        while (validAttacks != 0)
        {
            int dest = __builtin_ctzll(validAttacks);

            uint16_t move = src | dest << 6;
            list.addMove(move);

            validAttacks &= (validAttacks - 1);
        }

        squares &= (squares - 1);
    }
}

void serializeQueenMoves(const Board &board, movesList &list)
{
    const int queenDirections[8][2] = {
        {1, 1},
        {-1, 1},
        {1, -1},
        {-1, -1},
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}};

    myColor side = board.sideToMove;
    uint64_t squares = board.pieces[side][Queen];
    uint64_t sidePieces = side == White ? board.whitePieces : board.blackPieces;

    while (squares != 0)
    {
        int src = __builtin_ctzll(squares);
        uint64_t validAttacks = slidingMoves(src, board, queenDirections, 8) & ~sidePieces;
        while (validAttacks != 0)
        {
            int dest = __builtin_ctzll(validAttacks);

            uint16_t move = src | dest << 6;
            list.addMove(move);

            validAttacks &= (validAttacks - 1);
        }

        squares &= (squares - 1);
    }
}

void printBoard(const Board &board)
{
    const char symbolsW[] = {'P', 'N', 'B', 'R', 'K', 'Q'};
    const char symbolsB[] = {'p', 'n', 'b', 'r', 'k', 'q'};

    std::cout << "\n  +-----------------+\n";

    for (int rank = 7; rank >= 0; --rank)
    {
        std::cout << (rank + 1) << " | ";

        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 + file;
            uint64_t squareMask = 1ULL << square;
            char pieceChar = '.';

            bool pieceFound = false;
            for (int p = 0; p < 6; ++p)
            {
                if (board.pieces[White][p] & squareMask)
                {
                    pieceChar = symbolsW[p];
                    pieceFound = true;
                    break;
                }
                if (board.pieces[Black][p] & squareMask)
                {
                    pieceChar = symbolsB[p];
                    pieceFound = true;
                    break;
                }
            }

            std::cout << pieceChar << ' ';
        }
        std::cout << "|\n";
    }
    std::cout << "  +-----------------+\n";
    std::cout << "    a b c d e f g h\n\n";
    std::cout << "Side to move: " << (board.sideToMove == White ? "White" : "Black");
}

movesList generateMoves(const Board &board)
{

    // uint16_t move: bits: 0-5 source, 6-11 destination, 12-13 promotion piece, 13-16 flags (en passsant, castle, double push)

    movesList moves;

    serializePawnMoves(board, moves);
    serializeKnightMoves(board, moves);
    serializeKingMoves(board, moves);
    serializeRookMoves(board, moves);
    serializeBishopMoves(board, moves);
    serializeQueenMoves(board, moves);

    return moves;
}

int getPieceAtSquare(const Board &board, int square)
{
    uint64_t mask = 1ULL << square;

    for (int piece = 0; piece < 6; ++piece)
    {
        if (board.pieces[board.sideToMove][piece] & mask)
        {
            return piece;
        }
    }
    return -1;
}

bool isInCheck(const Board &board)
{
    // run after move is made, but before finalizing move

    Board copy = board;

    myColor side = copy.sideToMove;
    myColor opponent = (side == White) ? Black : White;

    copy.sideToMove = opponent;

    movesList moves = generateMoves(copy);

    copy.sideToMove = side;

    uint64_t kingSrc = copy.pieces[side][King];

    uint64_t pawns = copy.pieces[opponent][Pawn];
    uint64_t pawnAttacks = 0ULL;

    if (opponent == White)
    {
        pawnAttacks = ((pawns << 7) & 0x7F7F7F7F7F7F7F7FULL) | ((pawns << 9) & 0xFEFEFEFEFEFEFEFEULL);
    }
    else
    {
        pawnAttacks = ((pawns >> 9) & 0x7F7F7F7F7F7F7F7FULL) | ((pawns >> 7) & 0xFEFEFEFEFEFEFEFEULL);
    }
    if (pawnAttacks & kingSrc)
    {
        return true;
    }

    for (int i = 0; i < moves.count; ++i)
    {
        int destination = (moves.moves[i] >> 6) & 0x3f;
        uint64_t destMask = 1ULL << destination;

        int src = (moves.moves[i] & 0x3F);

        if (getPieceAtSquare(copy, src) == Pawn)
        {
            continue;
        }

        if (kingSrc & destMask)
        {
            return true;
        }
    }
    return false;
}

bool makeMove(Board &board, uint16_t move)
{
    Board boardCopy = board;

    int src = move & 0x3F;
    int dest = (move >> 6) & 0x3F;

    uint64_t srcMask = 1ULL << src;
    uint64_t destMask = 1ULL << dest;

    myColor movingSide = board.sideToMove;
    myColor enemySide = movingSide == White ? Black : White;

    int movingPiece = -1;

    for (int i = 0; i < 6; ++i)
    {
        if (board.pieces[movingSide][i] & srcMask)
        {
            movingPiece = i;
            break;
        }
    }
    if (movingPiece == -1)
    {
        return false;
    }

    uint64_t sidePieces = (movingSide == White) ? board.whitePieces : board.blackPieces;

    if (sidePieces & destMask)
    {
        return false;
    }

    board.pieces[movingSide][movingPiece] &= ~srcMask;

    for (int i = 0; i < 6; ++i)
    {
        if (board.pieces[enemySide][i] & destMask)
        {
            board.pieces[enemySide][i] &= ~destMask;
            break;
        }
    }

    board.pieces[movingSide][movingPiece] |= destMask;

    board.updateOccupancy();

    if (isInCheck(board))
    {
        board = boardCopy;
        return false;
    }
    else
    {
        board.sideToMove = enemySide;

        return true;
    }
}

int cordToMove(const std::string &input)
{
    if (input.length() != 2)
    {
        return -1;
    }

    char startFile = input[0];
    char startRow = input[1];

    if (startFile < 'a' || startFile > 'h')
    {
        return -1;
    }
    if (startRow < '1' || startRow > '8')
    {
        return -1;
    }

    int fileIndex = startFile - 'a';
    int rowIndex = startRow - '1';

    return rowIndex * 8 + fileIndex;
}

bool findMove(const movesList moves, int source, int dest, uint16_t &foundMove)
{
    for (int i = 0; i < moves.count; ++i)
    {
        uint16_t move = moves.moves[i];

        int src = move & 0x3F;
        int destination = (move >> 6) & 0x3f;

        if (src == source && dest == destination)
        {
            foundMove = move;
            return true;
        }
    }
    return false;
}

void drawBoard(int squareSize)
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            bool lightSquare = ((i + j) % 2 == 0);
            Color color = lightSquare ? Color{240, 217, 181, 255} : Color{181, 136, 99, 255};

            int screenR = 7 - i;

            DrawRectangle(
                j * squareSize,
                screenR * squareSize,
                squareSize,
                squareSize,
                color);
        }
    }
}

void drawPieces(const Board &board, const PieceTexture &pieceTexture, int squareSize)
{
    for (int side = 0; side < 2; ++side)
    {
        for (int piece = 0; piece < 6; ++piece)
        {
            uint64_t squares = board.pieces[side][piece];

            while (squares != 0)
            {
                int square = __builtin_ctzll(squares);
                int file = square % 8;
                int rank = square / 8;

                int screenRank = 7 - rank;

                Texture2D texture = pieceTexture.textures[side][piece];

                Rectangle source = {
                    0.0f,
                    0.0f,
                    static_cast<float>(texture.width),
                    static_cast<float>(texture.height)};

                float padding = squareSize * 0.08f;

                Rectangle dest = {
                    file * static_cast<float>(squareSize) + padding,
                    screenRank * static_cast<float>(squareSize) + padding,
                    squareSize - padding * 2,
                    squareSize - padding * 2,
                };

                DrawTexturePro(
                    texture, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

                squares &= squares - 1;
            }
        }
    }
}

void drawSelectedSquare(int square, int squareSize)
{
    if (square == -1)
    {
        return;
    }

    int file = square % 8;
    int rank = square / 8;

    int realRank = 7 - rank;

    DrawRectangleLinesEx(
        Rectangle{
            static_cast<float>(file * squareSize),
            static_cast<float>(realRank * squareSize),
            static_cast<float>(squareSize),
            static_cast<float>(squareSize)},
        5.0f, YELLOW);
}

int mouseToSquare(int x, int y, int squareSize)
{
    int file = x / squareSize;
    int rank = y / squareSize;

    if (file < 0 || file >= 8)
    {
        return -1;
    }
    if (rank < 0 || rank > 8)
    {
        return -1;
    }

    int rankReal = 7 - rank;

    return rankReal * 8 + file;
}

void unloadTextures(PieceTexture textures)
{
    for (int side = 0; side < 2; ++side)
    {
        for (int piece = 0; piece < 6; ++piece)
        {
            UnloadTexture(textures.textures[side][piece]);
        }
    }
}

bool hasLegalMove(const Board &board)
{
    movesList moves = generateMoves(board);

    for (int i = 0; i < moves.count; ++i)
    {
        Board copy = board;
        if (makeMove(copy, moves.moves[i]))
        {
            return true;
        }
    }
    return false;
}

bool isCheckmate(const Board &board)
{
    if (!isInCheck(board))
    {
        return false;
    }

    return !hasLegalMove(board);
}

bool isStalemate(const Board &board)
{
    if (isInCheck(board))
    {
        return false;
    }

    return !hasLegalMove(board);
}

int main()
{
    knightLookup();
    kingLookup();
    Board chessBoard;
    movesList moves;

    const int squareSize = 100;
    const int boardSize = squareSize * 8;

    int selectedSqaure = -1;

    InitWindow(boardSize, boardSize, "Chess");
    SetTargetFPS(60);

    PieceTexture pieceTexture = loadPieceTextures();

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 mouse = GetMousePosition();
            int clickedSquare = mouseToSquare(static_cast<int>(mouse.x), static_cast<int>(mouse.y), squareSize);

            if (clickedSquare != -1)
            {
                if (selectedSqaure == -1)
                {
                    selectedSqaure = clickedSquare;
                }
                else
                {
                    moves = generateMoves(chessBoard);
                    uint16_t foundMove = 0;

                    if (findMove(moves, selectedSqaure, clickedSquare, foundMove))
                    {
                        if (makeMove(chessBoard, foundMove))
                        {
                            if (isCheckmate(chessBoard))
                            {
                                std::cout << "\n Checkmate \n";
                            }
                            else if (isStalemate(chessBoard))
                            {
                                std::cout << "\n Stalemate \n";
                            }
                        }
                    }
                    selectedSqaure = -1;
                }
            }
        }
        BeginDrawing();

        ClearBackground(RAYWHITE);

        drawBoard(squareSize);
        drawSelectedSquare(selectedSqaure, squareSize);
        drawPieces(chessBoard, pieceTexture, squareSize);

        EndDrawing();
    };

    unloadTextures(pieceTexture);
    CloseWindow();
    return 0;
}