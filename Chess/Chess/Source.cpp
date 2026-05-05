#include "Header.h"
#include <iostream>
using namespace std;

// ═══════════════════════════════════════════════
//  MOVELIST
// ═══════════════════════════════════════════════
MoveList::MoveList() : count(0) {
    for (int i = 0; i < MAX_MOVES; i++)
        squares[i] = nullptr;
}

void MoveList::add(Square* sq) {
    if (sq && count < MAX_MOVES)
        squares[count++] = sq;
}

Square* MoveList::get(int index) const {
    if (index >= 0 && index < count)
        return squares[index];
    return nullptr;
}

bool MoveList::contains(Square* sq) const {
    for (int i = 0; i < count; i++)
        if (squares[i] == sq) return true;
    return false;
}


// ═══════════════════════════════════════════════
//  PIECE  (base)
// ═══════════════════════════════════════════════
Piece::Piece(Color color, PieceType type)
    : square(nullptr), color(color), type(type), moveCount(0) {
}

Color     Piece::getColor()     const { return color; }
PieceType Piece::getType()      const { return type; }
Square* Piece::getSquare()    const { return square; }
int       Piece::getMoveCount() const { return moveCount; }
bool      Piece::hasMoved()     const { return moveCount > 0; }

void Piece::setSquare(Square* sq) { square = sq; }
void Piece::incrementMoveCount() { moveCount++; }


// ═══════════════════════════════════════════════
//  PAWN
// ═══════════════════════════════════════════════
Pawn::Pawn(Color color) : Piece(color, PAWN), doubleStepTurn(-1) {}

char Pawn::getSymbol()          const { return (color == WHITE) ? 'P' : 'p'; }
bool Pawn::canEnPassant()       const { return doubleStepTurn >= 0; }
void Pawn::setDoubleStepTurn(int t) { doubleStepTurn = t; }
int  Pawn::getDoubleStepTurn()  const { return doubleStepTurn; }

MoveList Pawn::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();
    int dir = (color == WHITE) ? -1 : 1;   // white moves up (row--), black down

    // ── One step forward ──────────────────────
    int newRow = row + dir;
    if (board.inBounds(newRow, col)) {
        Square* front = board.getSquare(newRow, col);
        if (front->isEmpty()) {
            moves.add(front);

            // ── Two steps forward from starting rank ──
            int startRow = (color == WHITE) ? 6 : 1;
            if (row == startRow) {
                Square* twoFront = board.getSquare(row + 2 * dir, col);
                if (twoFront && twoFront->isEmpty())
                    moves.add(twoFront);
            }
        }
    }

    // ── Diagonal captures + en passant ────────
    int captureCols[2] = { col - 1, col + 1 };
    for (int i = 0; i < 2; i++) {
        int cc = captureCols[i];
        if (board.inBounds(newRow, cc)) {
            // Normal diagonal capture
            Square* diag = board.getSquare(newRow, cc);
            if (!diag->isEmpty() && diag->getPiece()->getColor() != color)
                moves.add(diag);

            // En passant: enemy Pawn on the same rank just double-stepped
            if (board.inBounds(row, cc)) {
                Square* neighbour = board.getSquare(row, cc);
                if (!neighbour->isEmpty()) {
                    Piece* p = neighbour->getPiece();
                    if (p->getType() == PAWN && p->getColor() != color) {
                        Pawn* enemy = static_cast<Pawn*>(p);
                        if (enemy->canEnPassant())
                            moves.add(diag);   // land on the square behind enemy
                    }
                }
            }
        }
    }

    return moves;
}


// ═══════════════════════════════════════════════
//  ROOK
// ═══════════════════════════════════════════════
Rook::Rook(Color color) : Piece(color, ROOK) {}

char Rook::getSymbol() const { return (color == WHITE) ? 'R' : 'r'; }

MoveList Rook::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

    // 4 orthogonal directions
    int dRow[4] = { -1,  1,  0,  0 };
    int dCol[4] = { 0,  0, -1,  1 };

    for (int d = 0; d < 4; d++) {
        int r = row + dRow[d];
        int c = col + dCol[d];
        while (board.inBounds(r, c)) {
            Square* sq = board.getSquare(r, c);
            if (sq->isEmpty()) {
                moves.add(sq);
            }
            else {
                if (sq->getPiece()->getColor() != color)
                    moves.add(sq);   // capture
                break;               // blocked
            }
            r += dRow[d];
            c += dCol[d];
        }
    }
    return moves;
}


// ═══════════════════════════════════════════════
//  KNIGHT
// ═══════════════════════════════════════════════
Knight::Knight(Color color) : Piece(color, KNIGHT) {}

char Knight::getSymbol() const { return (color == WHITE) ? 'N' : 'n'; }

MoveList Knight::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

    // All 8 L-shaped offsets
    int dRow[8] = { -2, -2, -1, -1,  1,  1,  2,  2 };
    int dCol[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };

    for (int i = 0; i < 8; i++) {
        int r = row + dRow[i];
        int c = col + dCol[i];
        if (board.inBounds(r, c)) {
            Square* sq = board.getSquare(r, c);
            if (sq->isEmpty() || sq->getPiece()->getColor() != color)
                moves.add(sq);
        }
    }
    return moves;
}


// ═══════════════════════════════════════════════
//  BISHOP
// ═══════════════════════════════════════════════
Bishop::Bishop(Color color) : Piece(color, BISHOP) {}

char Bishop::getSymbol() const { return (color == WHITE) ? 'B' : 'b'; }

MoveList Bishop::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

    // 4 diagonal directions
    int dRow[4] = { -1, -1,  1,  1 };
    int dCol[4] = { -1,  1, -1,  1 };

    for (int d = 0; d < 4; d++) {
        int r = row + dRow[d];
        int c = col + dCol[d];
        while (board.inBounds(r, c)) {
            Square* sq = board.getSquare(r, c);
            if (sq->isEmpty()) {
                moves.add(sq);
            }
            else {
                if (sq->getPiece()->getColor() != color)
                    moves.add(sq);
                break;
            }
            r += dRow[d];
            c += dCol[d];
        }
    }
    return moves;
}


// ═══════════════════════════════════════════════
//  QUEEN  (Rook + Bishop combined)
// ═══════════════════════════════════════════════
Queen::Queen(Color color) : Piece(color, QUEEN) {}

char Queen::getSymbol() const { return (color == WHITE) ? 'Q' : 'q'; }

MoveList Queen::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

    // All 8 directions
    int dRow[8] = { -1, -1, -1,  0,  0,  1,  1,  1 };
    int dCol[8] = { -1,  0,  1, -1,  1, -1,  0,  1 };

    for (int d = 0; d < 8; d++) {
        int r = row + dRow[d];
        int c = col + dCol[d];
        while (board.inBounds(r, c)) {
            Square* sq = board.getSquare(r, c);
            if (sq->isEmpty()) {
                moves.add(sq);
            }
            else {
                if (sq->getPiece()->getColor() != color)
                    moves.add(sq);
                break;
            }
            r += dRow[d];
            c += dCol[d];
        }
    }
    return moves;
}


// ═══════════════════════════════════════════════
//  KING
// ═══════════════════════════════════════════════
King::King(Color color) : Piece(color, KING) {}

char King::getSymbol() const { return (color == WHITE) ? 'K' : 'k'; }

MoveList King::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

    // One step in any of 8 directions
    int dRow[8] = { -1, -1, -1,  0,  0,  1,  1,  1 };
    int dCol[8] = { -1,  0,  1, -1,  1, -1,  0,  1 };

    for (int i = 0; i < 8; i++) {
        int r = row + dRow[i];
        int c = col + dCol[i];
        if (board.inBounds(r, c)) {
            Square* sq = board.getSquare(r, c);
            if (sq->isEmpty() || sq->getPiece()->getColor() != color)
                moves.add(sq);
        }
    }

    // ── Castling ──────────────────────────────
    // Requires: King has not moved, target Rook has not moved,
    //           all squares between them are empty.
    if (!hasMoved()) {
        int backRank = (color == WHITE) ? 7 : 0;

        // King-side (Rook on col 7, King lands on col 6)
        Square* rookSq = board.getSquare(backRank, 7);
        if (rookSq && !rookSq->isEmpty()) {
            Piece* rook = rookSq->getPiece();
            if (rook->getType() == ROOK &&
                rook->getColor() == color &&
                !rook->hasMoved() &&
                board.getSquare(backRank, 5)->isEmpty() &&
                board.getSquare(backRank, 6)->isEmpty())
            {
                moves.add(board.getSquare(backRank, 6));
            }
        }

        // Queen-side (Rook on col 0, King lands on col 2)
        rookSq = board.getSquare(backRank, 0);
        if (rookSq && !rookSq->isEmpty()) {
            Piece* rook = rookSq->getPiece();
            if (rook->getType() == ROOK &&
                rook->getColor() == color &&
                !rook->hasMoved() &&
                board.getSquare(backRank, 1)->isEmpty() &&
                board.getSquare(backRank, 2)->isEmpty() &&
                board.getSquare(backRank, 3)->isEmpty())
            {
                moves.add(board.getSquare(backRank, 2));
            }
        }
    }

    return moves;
}


// ═══════════════════════════════════════════════
//  SQUARE
// ═══════════════════════════════════════════════
Square::Square() : name(""), piece(nullptr), row(0), col(0) {}

Square::Square(string name, int row, int col)
    : name(name), piece(nullptr), row(row), col(col) {
}

string Square::getName()  const { return name; }
Piece* Square::getPiece() const { return piece; }
int    Square::getRow()   const { return row; }
int    Square::getCol()   const { return col; }
bool   Square::isEmpty()  const { return piece == nullptr; }

void Square::setPiece(Piece* p) { piece = p; if (p) p->setSquare(this); }
void Square::clearPiece() { piece = nullptr; }


// ═══════════════════════════════════════════════
//  BOARD
// ═══════════════════════════════════════════════
Board::Board() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char file = 'a' + j;
            char rank = '0' + (8 - i);
            string nm = "";
            nm += file;
            nm += rank;
            squares[i][j] = Square(nm, i, j);
        }
    }
}

Square* Board::getSquare(int row, int col) {
    if (!inBounds(row, col)) return nullptr;
    return &squares[row][col];
}

Square* Board::getSquare(const string& name) {
    if (name.size() < 2) return nullptr;
    int col = name[0] - 'a';
    int row = '8' - name[1];
    return getSquare(row, col);
}

bool Board::inBounds(int row, int col) const {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

void Board::placePiece(Piece* piece, int row, int col) {
    if (inBounds(row, col))
        squares[row][col].setPiece(piece);
}

void Board::removePiece(int row, int col) {
    if (inBounds(row, col))
        squares[row][col].clearPiece();
}

void Board::setupPieces() {
    // ── Black pieces (row 0 = rank 8) ────────
    placePiece(new Rook(BLACK), 0, 0);
    placePiece(new Knight(BLACK), 0, 1);
    placePiece(new Bishop(BLACK), 0, 2);
    placePiece(new Queen(BLACK), 0, 3);
    placePiece(new King(BLACK), 0, 4);
    placePiece(new Bishop(BLACK), 0, 5);
    placePiece(new Knight(BLACK), 0, 6);
    placePiece(new Rook(BLACK), 0, 7);
    for (int c = 0; c < 8; c++)
        placePiece(new Pawn(BLACK), 1, c);

    // ── White pieces (row 7 = rank 1) ────────
    placePiece(new Rook(WHITE), 7, 0);
    placePiece(new Knight(WHITE), 7, 1);
    placePiece(new Bishop(WHITE), 7, 2);
    placePiece(new Queen(WHITE), 7, 3);
    placePiece(new King(WHITE), 7, 4);
    placePiece(new Bishop(WHITE), 7, 5);
    placePiece(new Knight(WHITE), 7, 6);
    placePiece(new Rook(WHITE), 7, 7);
    for (int c = 0; c < 8; c++)
        placePiece(new Pawn(WHITE), 6, c);
}

void Board::display() const {
    cout << "\n    a  b  c  d  e  f  g  h\n";
    cout << "  +------------------------+\n";
    for (int i = 0; i < 8; i++) {
        cout << (8 - i) << " | ";
        for (int j = 0; j < 8; j++) {
            Piece* p = squares[i][j].getPiece();
            cout << (p ? p->getSymbol() : '.') << "  ";
        }
        cout << "| " << (8 - i) << "\n";
    }
    cout << "  +------------------------+\n";
    cout << "    a  b  c  d  e  f  g  h\n\n";
}


// ═══════════════════════════════════════════════
//  PLAYER
// ═══════════════════════════════════════════════
Player::Player() : color(WHITE), activeCount(0), capturedCount(0) {
    for (int i = 0; i < 16; i++) activePieces[i] = capturedPieces[i] = nullptr;
}

Player::Player(Color color) : color(color), activeCount(0), capturedCount(0) {
    for (int i = 0; i < 16; i++) activePieces[i] = capturedPieces[i] = nullptr;
}

void Player::addPiece(Piece* piece) {
    if (activeCount < 16)
        activePieces[activeCount++] = piece;
}

void Player::capturePiece(Piece* piece) {
    for (int i = 0; i < activeCount; i++) {
        if (activePieces[i] == piece) {
            activePieces[i] = activePieces[--activeCount];
            activePieces[activeCount] = nullptr;
            break;
        }
    }
    if (capturedCount < 16)
        capturedPieces[capturedCount++] = piece;
}

Color  Player::getColor()              const { return color; }
int    Player::getActiveCount()        const { return activeCount; }
int    Player::getCapturedCount()      const { return capturedCount; }
Piece* Player::getActivePiece(int i)   const { return (i < activeCount) ? activePieces[i] : nullptr; }
Piece* Player::getCapturedPiece(int i) const { return (i < capturedCount) ? capturedPieces[i] : nullptr; }


// ═══════════════════════════════════════════════
//  GAME
// ═══════════════════════════════════════════════
Game::Game()
    : white(WHITE), black(BLACK), currentTurn(&white),
    status(ONGOING), turnNumber(0) {
}

void Game::start() {
    board.setupPieces();
    display();
}

void Game::switchTurn() {
    currentTurn = (currentTurn == &white) ? &black : &white;
    turnNumber++;
}

State Game::getStatus() const { return status; }

void Game::display() const {
    board.display();
    cout << "Turn: " << ((currentTurn == &white) ? "WHITE" : "BLACK")
        << "  |  Move #" << turnNumber << "\n";
}

// checks if a king of given color is under attack
bool Game::isInCheck(Color color) {
    // find the king's square
    Square* kingSquare = nullptr;
    Player& me = (color == WHITE) ? white : black;
    for (int i = 0; i < me.getActiveCount(); i++) {
        Piece* p = me.getActivePiece(i);
        if (p->getType() == KING) {
            kingSquare = p->getSquare();
            break;
        }
    }
    if (!kingSquare) return false;

    // check if any enemy piece can reach the king
    Player& enemy = (color == WHITE) ? black : white;
    for (int i = 0; i < enemy.getActiveCount(); i++) {
        MoveList moves = enemy.getActivePiece(i)->availableMoves(board);
        if (moves.contains(kingSquare))
            return true;
    }
    return false;
}

// checks if the given color has at least one legal move
bool Game::hasAnyMoves(Color color) {
    Player& me = (color == WHITE) ? white : black;
    for (int i = 0; i < me.getActiveCount(); i++) {
        MoveList moves = me.getActivePiece(i)->availableMoves(board);
        if (moves.count > 0)
            return true;
    }
    return false;
}

void Game::updateStatus() {
    Color turn = currentTurn->getColor();
    bool inCheck = isInCheck(turn);
    bool hasMoves = hasAnyMoves(turn);

    if (inCheck && !hasMoves)
        status = CHECKMATE;
    else if (!inCheck && !hasMoves)
        status = STALEMATE;
    else if (inCheck)
        status = CHECK;
    else
        status = ONGOING;
}

//MOVE HISTORY FUNCTIONS

MoveHistory::MoveHistory() : moveCount(0) {}

void MoveHistory::recordMove(Piece* piece, Square* from, Square* to, bool isCapture, bool isCheck, bool isCheckmate) {
    if (moveCount >= 500) return;

    string notation = "";

    // Piece letter (pawns have none)
    char symbol = piece->getSymbol();
    if (symbol != 'P' && symbol != 'p')
        notation += toupper(symbol);

    // Capture
    if (isCapture) notation += "x";

    // Destination
    notation += to->getName();

    // Check / Checkmate
    if (isCheckmate)   notation += "#";
    else if (isCheck)  notation += "+";

    moves[moveCount++] = notation;
}

void MoveHistory::printHistory() const {
    for (int i = 0; i < moveCount; i += 2) {
        cout << (i / 2 + 1) << ". " << moves[i];
        if (i + 1 < moveCount)
            cout << "  " << moves[i + 1];
        cout << "\n";
    }
}

string MoveHistory::getLastMove() const {
    if (moveCount == 0) return "";
    return moves[moveCount - 1];
}

void MoveHistory::undoLast() {
    if (moveCount > 0)
        moves[--moveCount] = "";
}

int MoveHistory::getCount() const { return moveCount; }