#pragma once
#include <iostream>
#include <string>
using namespace std;

// Forward declarations
class Board;
class Square;
class Piece;
class Player;

enum State { ONGOING, CHECK, CHECKMATE, STALEMATE, DRAW_FIFTY_MOVE, DRAW_INSUFFICIENT, DRAW_AGREEMENT, RESIGNED };
enum Color { WHITE, BLACK };
enum PieceType { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };

// ─────────────────────────────────────────────
//  MOVELIST  – plain fixed-size array + count
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
    int       moveCount;

public:
    Piece(Color color, PieceType type);
    virtual ~Piece() {}

    // Non-copyable — raw square pointer makes copying unsafe
    Piece(const Piece&) = delete;
    Piece& operator=(const Piece&) = delete;

    virtual MoveList availableMoves(Board& board) const = 0;

    Color     getColor()     const;
    PieceType getType()      const;
    Square* getSquare()    const;
    int       getMoveCount() const;
    bool      hasMoved()     const;

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

    bool canEnPassant(int currentTurn) const;  // true only on the immediately following turn
    void setDoubleStepTurn(int turn);
    int  getDoubleStepTurn() const;

    char getSymbol() const override;
};

class Rook : public Piece {
public:
    Rook(Color color);
    MoveList availableMoves(Board& board) const override;
    char getSymbol() const override;
};

class Knight : public Piece {
public:
    Knight(Color color);
    MoveList availableMoves(Board& board) const override;
    char getSymbol() const override;
};

class Bishop : public Piece {
public:
    Bishop(Color color);
    MoveList availableMoves(Board& board) const override;
    char getSymbol() const override;
};

class Queen : public Piece {
public:
    Queen(Color color);
    MoveList availableMoves(Board& board) const override;
    char getSymbol() const override;
};

class King : public Piece {
public:
    King(Color color);
    MoveList availableMoves(Board& board) const override;
    char getSymbol() const override;
};

// ─────────────────────────────────────────────
//  SQUARE
// ─────────────────────────────────────────────
class Square {
    string name;
    Piece* piece;
    int    row;
    int    col;

public:
    Square();
    Square(string name, int row, int col);

    string  getName()  const;
    Piece* getPiece() const;
    int     getRow()   const;
    int     getCol()   const;
    bool    isEmpty()  const;

    void setPiece(Piece* p);
    void clearPiece();
};

// ─────────────────────────────────────────────
//  BOARD
// ─────────────────────────────────────────────
class Board {
    Square squares[8][8];
    // All allocated pieces tracked here for safe cleanup
    Piece* allPieces[64];
    int    pieceCount;

public:
    Board();
    ~Board();

    // Non-copyable — squares hold raw Piece* pointers
    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    Square* getSquare(int row, int col);
    Square* getSquare(const string& name);

    bool inBounds(int row, int col) const;
    // Attack check that skips castling to avoid infinite recursion
    bool isSquareAttackedBy(Square* sq, Color attackerColor, bool skipCastling = true);

    void placePiece(Piece* piece, int row, int col);
    void removePiece(int row, int col);
    void registerPiece(Piece* p);   // adds to allPieces for later cleanup

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
    // Removes piece from active list only (Board owns memory)
    void removePiece(Piece* piece);
    // Records enemy piece as captured trophy (does NOT free memory)
    void recordCapture(Piece* piece);

    Color  getColor()                  const;
    int    getActiveCount()            const;
    int    getCapturedCount()          const;
    Piece* getActivePiece(int index)   const;
    Piece* getCapturedPiece(int index) const;
};

// ─────────────────────────────────────────────
//  MOVE HISTORY
// ─────────────────────────────────────────────
class MoveHistory {
    static const int MAX_MOVES = 1000;
    string moves[MAX_MOVES];
    int    moveCount;
public:
    MoveHistory();
    void   recordMove(Piece* piece, Square* from, Square* to,
        bool isCapture, bool isCheck, bool isCheckmate,
        const string& promotionSuffix = "");
    void   printHistory() const;
    string getLastMove()  const;
    void   undoLast();
    int    getCount()     const;
};

// ─────────────────────────────────────────────
//  INPUT HANDLER
// ─────────────────────────────────────────────
class InputHandler {
public:
    // Returns false on quit; sets from=to=nullptr for draw offer
    bool      getMove(Board& board, Square*& from, Square*& to);
    PieceType getPromotion();
    bool      wantsQuit(const string& input);
    bool      offersDraw(const string& input);
};

// ─────────────────────────────────────────────
//  GAME
// ─────────────────────────────────────────────
class Game {
    Board        board;
    Player       white;
    Player       black;
    Player* currentTurn;
    State        status;
    int          turnNumber;
    int          halfMoveClock;
    MoveHistory  history;
    InputHandler input;

public:
    Game();

    // ── Console interface ────────────────────
    void  start();
    void  run();
    void  switchTurn();
    void  display()      const;

    // ── Shared / GUI interface ───────────────
    State    getStatus()       const;
    void     updateStatus();
    bool     makeMove(Square* from, Square* to, PieceType promotion = QUEEN);
    bool     isInCheck(Color color);
    bool     hasAnyMoves(Color color);
    Color    getCurrentTurnColor() const { return currentTurn->getColor(); }
    Board& getBoard() { return board; }
    Player& getWhitePlayer() { return white; }
    Player& getBlackPlayer() { return black; }
    // Returns all legal destinations for the piece on 'from' (empty MoveList if none)
    MoveList getLegalMovesFrom(Square* from);
    // Convenience: reset to a fresh game
    void     reset();
    void offersDraw();
    void resign();

private:
    bool wouldLeaveKingInCheck(Square* from, Square* to, Color color);
    void handlePromotion(Square* sq, Color color, PieceType promotion);
    bool isInsufficientMaterial();
};