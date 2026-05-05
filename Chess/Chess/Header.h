#pragma once
#pragma once
#include <iostream>
#include <string>
using namespace std;

// Forward declarations
class Board;
class Square;
class Piece;
class Player;

enum State { ONGOING, CHECK, CHECKMATE, STALEMATE, DRAW };
enum Color { WHITE, BLACK };
enum PieceType { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };

// ─────────────────────────────────────────────
//  MOVELIST  – plain fixed-size array + count
//  Replaces vector<Square*> with no dynamic allocation.
//  32 slots covers the maximum possible moves for any piece (Queen = 27).
// ─────────────────────────────────────────────
struct MoveList {
    static const int MAX_MOVES = 32;
    Square* squares[MAX_MOVES];
    int     count;

    MoveList();
    void    add(Square* sq);
    Square* get(int index)    const;
    bool    contains(Square* sq) const;
};

// ─────────────────────────────────────────────
//  PIECE  (abstract base)
// ─────────────────────────────────────────────
class Piece {
protected:
    Square* square;
    Color     color;
    PieceType type;
    int       moveCount;   // total moves made; used for castling / en passant

public:
    Piece(Color color, PieceType type);
    virtual ~Piece() {}

    virtual MoveList availableMoves(Board& board) const = 0;

    Color     getColor()     const;
    PieceType getType()      const;
    Square* getSquare()    const;
    int       getMoveCount() const;
    bool      hasMoved()     const;   // true when moveCount > 0

    void setSquare(Square* sq);
    void incrementMoveCount();

    virtual char getSymbol() const = 0;
};

// ─────────────────────────────────────────────
//  CONCRETE PIECES
// ─────────────────────────────────────────────
class Pawn : public Piece {
    int doubleStepTurn;   // turn number of last double-step (-1 = never)

public:
    Pawn(Color color);

    MoveList availableMoves(Board& board) const override;

    bool canEnPassant()              const;   // true when doubleStepTurn >= 0
    void setDoubleStepTurn(int turn);
    int  getDoubleStepTurn()         const;

    char getSymbol() const override;          // 'P' (white) / 'p' (black)
};

class Rook : public Piece {
public:
    Rook(Color color);

    MoveList availableMoves(Board& board) const override;
    // hasMoved() == false  →  castling still an option

    char getSymbol() const override;          // 'R' / 'r'
};

class Knight : public Piece {
public:
    Knight(Color color);

    MoveList availableMoves(Board& board) const override;

    char getSymbol() const override;          // 'N' / 'n'
};

class Bishop : public Piece {
public:
    Bishop(Color color);

    MoveList availableMoves(Board& board) const override;

    char getSymbol() const override;          // 'B' / 'b'
};

class Queen : public Piece {
public:
    Queen(Color color);

    MoveList availableMoves(Board& board) const override;

    char getSymbol() const override;          // 'Q' / 'q'
};

class King : public Piece {
public:
    King(Color color);

    MoveList availableMoves(Board& board) const override;
    // hasMoved() == false  →  castling still an option

    char getSymbol() const override;          // 'K' / 'k'
};

// ─────────────────────────────────────────────
//  SQUARE
// ─────────────────────────────────────────────
class Square {
    string name;
    Piece* piece;
    int    row;    // 0-7  (0 = rank 8)
    int    col;    // 0-7  (0 = file a)

public:
    Square();
    Square(string name, int row, int col);

    string getName()  const;
    Piece* getPiece() const;
    int    getRow()   const;
    int    getCol()   const;
    bool   isEmpty()  const;

    void setPiece(Piece* p);
    void clearPiece();
};

// ─────────────────────────────────────────────
//  BOARD
// ─────────────────────────────────────────────
class Board {
    Square squares[8][8];

public:
    Board();

    Square* getSquare(int row, int col);
    Square* getSquare(const string& name);

    bool inBounds(int row, int col) const;

    void placePiece(Piece* piece, int row, int col);
    void removePiece(int row, int col);

    void setupPieces();
    void display() const;
};

// ─────────────────────────────────────────────
//  PLAYER
// ─────────────────────────────────────────────
class Player {
    Color  color;
    Piece* activePieces[16];
    Piece* capturedPieces[16];
    int    activeCount;
    int    capturedCount;

public:
    Player();
    Player(Color color);

    void addPiece(Piece* piece);
    void capturePiece(Piece* piece);

    Color  getColor()                  const;
    int    getActiveCount()            const;
    int    getCapturedCount()          const;
    Piece* getActivePiece(int index)   const;
    Piece* getCapturedPiece(int index) const;
};

//MOVE HISTORY
class MoveHistory {
    string moves[500];
    int moveCount;
public:
    MoveHistory();
    void recordMove(Piece* piece, Square* from, Square* to, bool isCapture, bool isCheck, bool isCheckmate);
    void printHistory() const;
    string getLastMove() const;
    void undoLast();
    int getCount() const;
};

// handles all player input, swap this out for Windows.h later
class InputHandler {
public:
    // returns two squares from input like "e2 e4"
    // returns false if input is invalid
    bool getMove(Board& board, Square*& from, Square*& to);

    // returns promotion choice from player
    PieceType getPromotion();

    // returns true if player wants to quit
    bool wantsQuit(const string& input);
};

// ─────────────────────────────────────────────
//  GAME
// ─────────────────────────────────────────────
class Game {
    Board   board;
    Player  white;
    Player  black;
    Player* currentTurn;
    State   status;
    int     turnNumber;
    MoveHistory history;
    InputHandler input;
public:
    Game();

    void  start();
    void run();
    void  switchTurn();
    State getStatus()    const;
    void  updateStatus();
    void  display()      const;
    bool  makeMove(Square* from, Square* to, PieceType promotion = QUEEN);
    bool isInCheck(Color color);
    bool hasAnyMoves(Color color);

private:
    bool wouldLeaveKingInCheck(Square* from, Square* to, Color color);
    void handlePromotion(Square* sq, Color color, PieceType promotion);
};
