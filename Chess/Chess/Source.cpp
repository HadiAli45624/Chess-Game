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
    : square(nullptr), color(color), type(type), moveCount(0) {}

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

char Pawn::getSymbol()                    const { return (color == WHITE) ? 'P' : 'p'; }
// FIX: en passant is only legal on the turn immediately after the double step
bool Pawn::canEnPassant(int currentTurn)  const { return doubleStepTurn == currentTurn - 1; }
void Pawn::setDoubleStepTurn(int t) { doubleStepTurn = t; }
int  Pawn::getDoubleStepTurn()            const { return doubleStepTurn; }

MoveList Pawn::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();
    int dir = (color == WHITE) ? -1 : 1;

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
        if (!board.inBounds(newRow, cc)) continue;

        Square* diag = board.getSquare(newRow, cc);

        // Normal diagonal capture
        if (!diag->isEmpty() && diag->getPiece()->getColor() != color) {
            moves.add(diag);
            continue;  // can't also en-passant to the same square
        }

        // FIX: en passant — only if diag is empty (the landing square behind enemy)
        if (diag->isEmpty() && board.inBounds(row, cc)) {
            Square* neighbour = board.getSquare(row, cc);
            if (!neighbour->isEmpty()) {
                Piece* p = neighbour->getPiece();
                if (p->getType() == PAWN && p->getColor() != color) {
                    // NOTE: availableMoves has no access to turnNumber here;
                    // canEnPassant(turn) is validated in makeMove / wouldLeaveKingInCheck
                    // via the Game layer. We just flag the geometric possibility here
                    // and let Game::makeMove confirm the turn-based legality.
                    Pawn* enemy = static_cast<Pawn*>(p);
                    if (enemy->getDoubleStepTurn() >= 0)
                        moves.add(diag);
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
//  KNIGHT
// ═══════════════════════════════════════════════
Knight::Knight(Color color) : Piece(color, KNIGHT) {}

char Knight::getSymbol() const { return (color == WHITE) ? 'N' : 'n'; }

MoveList Knight::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

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
//  QUEEN
// ═══════════════════════════════════════════════
Queen::Queen(Color color) : Piece(color, QUEEN) {}

char Queen::getSymbol() const { return (color == WHITE) ? 'Q' : 'q'; }

MoveList Queen::availableMoves(Board& board) const {
    MoveList moves;
    if (!square) return moves;

    int row = square->getRow();
    int col = square->getCol();

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
    // FIX: pass skipCastling=true to isSquareAttackedBy to avoid infinite
    //      recursion (King asks if square is attacked → attack check calls
    //      availableMoves on enemy King → enemy King asks about castling → ...).
    if (!hasMoved()) {
        int   backRank = (color == WHITE) ? 7 : 0;
        Color enemy = (color == WHITE) ? BLACK : WHITE;

        // King must not currently be in check
        if (!board.isSquareAttackedBy(board.getSquare(backRank, 4), enemy, true)) {

            // King-side
            Square* rookSq = board.getSquare(backRank, 7);
            if (rookSq && !rookSq->isEmpty()) {
                Piece* rook = rookSq->getPiece();
                if (rook->getType() == ROOK &&
                    rook->getColor() == color &&
                    !rook->hasMoved() &&
                    board.getSquare(backRank, 5)->isEmpty() &&
                    board.getSquare(backRank, 6)->isEmpty() &&
                    !board.isSquareAttackedBy(board.getSquare(backRank, 5), enemy, true) &&
                    !board.isSquareAttackedBy(board.getSquare(backRank, 6), enemy, true))
                {
                    moves.add(board.getSquare(backRank, 6));
                }
            }

            // Queen-side
            rookSq = board.getSquare(backRank, 0);
            if (rookSq && !rookSq->isEmpty()) {
                Piece* rook = rookSq->getPiece();
                if (rook->getType() == ROOK &&
                    rook->getColor() == color &&
                    !rook->hasMoved() &&
                    board.getSquare(backRank, 1)->isEmpty() &&
                    board.getSquare(backRank, 2)->isEmpty() &&
                    board.getSquare(backRank, 3)->isEmpty() &&
                    !board.isSquareAttackedBy(board.getSquare(backRank, 3), enemy, true) &&
                    !board.isSquareAttackedBy(board.getSquare(backRank, 2), enemy, true))
                {
                    moves.add(board.getSquare(backRank, 2));
                }
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
    : name(name), piece(nullptr), row(row), col(col) {}

string  Square::getName()  const { return name; }
Piece* Square::getPiece() const { return piece; }
int     Square::getRow()   const { return row; }
int     Square::getCol()   const { return col; }
bool    Square::isEmpty()  const { return piece == nullptr; }

void Square::setPiece(Piece* p) { piece = p; if (p) p->setSquare(this); }
void Square::clearPiece() { piece = nullptr; }


// ═══════════════════════════════════════════════
//  BOARD
// ═══════════════════════════════════════════════
Board::Board() : pieceCount(0) {
    for (int i = 0; i < 64; i++) allPieces[i] = nullptr;

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

// FIX: Board destructor frees all allocated pieces
Board::~Board() {
    for (int i = 0; i < pieceCount; i++) {
        delete allPieces[i];
        allPieces[i] = nullptr;
    }
}

// FIX: track every allocated piece so the destructor can free them
void Board::registerPiece(Piece* p) {
    if (p && pieceCount < 64)
        allPieces[pieceCount++] = p;
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
    // Helper lambda to allocate, register, and place a piece
    auto place = [&](Piece* p, int r, int c) {
        registerPiece(p);
        placePiece(p, r, c);
        };

    // Black pieces (row 0 = rank 8)
    place(new Rook(BLACK), 0, 0);
    place(new Knight(BLACK), 0, 1);
    place(new Bishop(BLACK), 0, 2);
    place(new Queen(BLACK), 0, 3);
    place(new King(BLACK), 0, 4);
    place(new Bishop(BLACK), 0, 5);
    place(new Knight(BLACK), 0, 6);
    place(new Rook(BLACK), 0, 7);
    for (int c = 0; c < 8; c++)
        place(new Pawn(BLACK), 1, c);

    // White pieces (row 7 = rank 1)
    place(new Rook(WHITE), 7, 0);
    place(new Knight(WHITE), 7, 1);
    place(new Bishop(WHITE), 7, 2);
    place(new Queen(WHITE), 7, 3);
    place(new King(WHITE), 7, 4);
    place(new Bishop(WHITE), 7, 5);
    place(new Knight(WHITE), 7, 6);
    place(new Rook(WHITE), 7, 7);
    for (int c = 0; c < 8; c++)
        place(new Pawn(WHITE), 6, c);
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

// FIX: skipCastling parameter prevents infinite recursion:
//      King::availableMoves → isSquareAttackedBy → King::availableMoves → ...
bool Board::isSquareAttackedBy(Square* sq, Color attackerColor, bool skipCastling) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Square* s = getSquare(r, c);
            if (s->isEmpty()) continue;
            Piece* p = s->getPiece();
            if (p->getColor() != attackerColor) continue;

            // When checking castling safety, skip the enemy King's castling moves
            // to avoid the recursion spiral described above.
            if (skipCastling && p->getType() == KING) {
                // Manually check the 8 one-step king moves only
                int pr = s->getRow(), pc = s->getCol();
                int dRow[8] = { -1,-1,-1, 0, 0, 1, 1, 1 };
                int dCol[8] = { -1, 0, 1,-1, 1,-1, 0, 1 };
                for (int i = 0; i < 8; i++) {
                    int nr = pr + dRow[i], nc = pc + dCol[i];
                    if (inBounds(nr, nc) && getSquare(nr, nc) == sq)
                        return true;
                }
                continue;
            }

            MoveList attacks = p->availableMoves(*this);
            if (attacks.contains(sq)) return true;
        }
    }
    return false;
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

// FIX: renamed from capturePiece; only removes from active list
//      Memory is owned by Board and freed in Board::~Board()
void Player::removePiece(Piece* piece) {
    for (int i = 0; i < activeCount; i++) {
        if (activePieces[i] == piece) {
            activePieces[i] = activePieces[--activeCount];
            activePieces[activeCount] = nullptr;
            return;
        }
    }
}

// FIX: records an enemy piece as a captured trophy (no memory management here)
void Player::recordCapture(Piece* piece) {
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
    status(ONGOING), turnNumber(0), halfMoveClock(0) {}

void Game::start() {
    board.setupPieces();

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Square* sq = board.getSquare(r, c);
            if (!sq->isEmpty()) {
                Piece* p = sq->getPiece();
                if (p->getColor() == WHITE) white.addPiece(p);
                else                        black.addPiece(p);
            }
        }
    }

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

bool Game::isInCheck(Color color) {
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

    Color  enemyColor = (color == WHITE) ? BLACK : WHITE;
    return board.isSquareAttackedBy(kingSquare, enemyColor, true);
}

bool Game::hasAnyMoves(Color color) {
    Player& me = (color == WHITE) ? white : black;
    for (int i = 0; i < me.getActiveCount(); i++) {
        Piece* piece = me.getActivePiece(i);
        MoveList moves = piece->availableMoves(board);
        for (int j = 0; j < moves.count; j++) {
            Square* from = piece->getSquare();
            Square* to = moves.get(j);
            if (!wouldLeaveKingInCheck(from, to, color))
                return true;
        }
    }
    return false;
}

bool Game::isInsufficientMaterial() {
    int whiteCount = white.getActiveCount();
    int blackCount = black.getActiveCount();

    if (whiteCount == 1 && blackCount == 1) return true;

    if (whiteCount == 2 && blackCount == 1) {
        for (int i = 0; i < white.getActiveCount(); i++) {
            PieceType t = white.getActivePiece(i)->getType();
            if (t == BISHOP || t == KNIGHT) return true;
        }
    }
    if (blackCount == 2 && whiteCount == 1) {
        for (int i = 0; i < black.getActiveCount(); i++) {
            PieceType t = black.getActivePiece(i)->getType();
            if (t == BISHOP || t == KNIGHT) return true;
        }
    }

    if (whiteCount == 2 && blackCount == 2) {
        Square* wBishopSq = nullptr;
        Square* bBishopSq = nullptr;
        for (int i = 0; i < white.getActiveCount(); i++)
            if (white.getActivePiece(i)->getType() == BISHOP)
                wBishopSq = white.getActivePiece(i)->getSquare();
        for (int i = 0; i < black.getActiveCount(); i++)
            if (black.getActivePiece(i)->getType() == BISHOP)
                bBishopSq = black.getActivePiece(i)->getSquare();

        if (wBishopSq && bBishopSq) {
            bool wLight = (wBishopSq->getRow() + wBishopSq->getCol()) % 2 == 0;
            bool bLight = (bBishopSq->getRow() + bBishopSq->getCol()) % 2 == 0;
            if (wLight == bLight) return true;
        }
    }

    return false;
}

void Game::updateStatus() {
    Color turn = currentTurn->getColor();
    bool inCheck = isInCheck(turn);
    bool hasMoves = hasAnyMoves(turn);

    if (inCheck && !hasMoves) status = CHECKMATE;
    else if (!inCheck && !hasMoves) status = STALEMATE;
    else if (halfMoveClock >= 100)  status = DRAW_FIFTY_MOVE;
    else if (isInsufficientMaterial()) status = DRAW_INSUFFICIENT;
    else if (inCheck)               status = CHECK;
    else                            status = ONGOING;
}

// FIX: handlePromotion no longer calls capturePiece on the old pawn.
//      It removes the pawn from active pieces, allocates and registers
//      the new piece, then places it. The old pawn's memory is owned
//      by Board and will be freed by Board::~Board().
void Game::handlePromotion(Square* sq, Color color, PieceType promotion) {
    Piece* newPiece = nullptr;
    switch (promotion) {
    case QUEEN:  newPiece = new Queen(color);  break;
    case ROOK:   newPiece = new Rook(color);   break;
    case BISHOP: newPiece = new Bishop(color); break;
    case KNIGHT: newPiece = new Knight(color); break;
    default:     newPiece = new Queen(color);  break;
    }
    board.registerPiece(newPiece);

    Piece* oldPawn = sq->getPiece();

    // Remove the pawn from the player's active list (Board still owns its memory)
    Player& me = (color == WHITE) ? white : black;
    me.removePiece(oldPawn);

    // Place and register the new piece
    sq->setPiece(newPiece);
    me.addPiece(newPiece);
}

// FIX: after the simulation undo, manually restore the moving piece's square
//      pointer so it stays accurate regardless of setPiece's side-effect.
bool Game::wouldLeaveKingInCheck(Square* from, Square* to, Color color) {
    Piece* movingPiece = from->getPiece();
    Piece* capturedPiece = to->getPiece();

    // Detect en passant: pawn moves diagonally to an empty square
    Square* epSquare = nullptr;
    Piece* epPawn = nullptr;
    if (movingPiece && movingPiece->getType() == PAWN &&
        from->getCol() != to->getCol() && capturedPiece == nullptr)
    {
        epSquare = board.getSquare(from->getRow(), to->getCol());
        if (epSquare) {
            epPawn = epSquare->getPiece();
            epSquare->clearPiece();
        }
    }

    // Temporarily apply the move (setPiece updates square pointers)
    to->setPiece(movingPiece);
    from->clearPiece();

    bool inCheck = isInCheck(color);

    // Undo the move
    from->setPiece(movingPiece);   // restores movingPiece->square = from
    to->clearPiece();
    if (capturedPiece)
        to->setPiece(capturedPiece);   // restores capturedPiece->square = to

    // Restore en-passant pawn
    if (epSquare && epPawn)
        epSquare->setPiece(epPawn);

    return inCheck;
}

bool Game::makeMove(Square* from, Square* to, PieceType promotion) {
    if (!from || !to) return false;

    Piece* piece = from->getPiece();
    if (!piece) return false;

    if (piece->getColor() != currentTurn->getColor()) return false;

    MoveList moves = piece->availableMoves(board);
    if (!moves.contains(to)) return false;

    if (wouldLeaveKingInCheck(from, to, currentTurn->getColor())) return false;

    // FIX: validate en-passant legality at the Game level (turn check)
    if (piece->getType() == PAWN &&
        from->getCol() != to->getCol() && to->isEmpty())
    {
        Square* epNeighbour = board.getSquare(from->getRow(), to->getCol());
        if (!epNeighbour || epNeighbour->isEmpty()) return false;
        Piece* epPiece = epNeighbour->getPiece();
        if (epPiece->getType() != PAWN || epPiece->getColor() == piece->getColor())
            return false;
        Pawn* epEnemy = static_cast<Pawn*>(epPiece);
        // Must have double-stepped exactly last turn
        if (epEnemy->getDoubleStepTurn() != turnNumber - 1) return false;
    }

    // Capture enemy piece if present
    bool isCapture = false;
    if (!to->isEmpty()) {
        Piece* captured = to->getPiece();
        Player& enemy = (currentTurn->getColor() == WHITE) ? black : white;
        Player& me = (currentTurn->getColor() == WHITE) ? white : black;
        // FIX: removePiece from active, recordCapture as trophy; Board frees memory
        enemy.removePiece(captured);
        me.recordCapture(captured);
        isCapture = true;
    }

    // Fifty-move rule clock
    if (isCapture || piece->getType() == PAWN)
        halfMoveClock = 0;
    else
        halfMoveClock++;

    // Execute move
    to->setPiece(piece);
    from->clearPiece();
    piece->incrementMoveCount();

    // Handle castling
    if (piece->getType() == KING) {
        int backRank = (piece->getColor() == WHITE) ? 7 : 0;
        if (to->getCol() == 6) {   // king-side
            Square* rookFrom = board.getSquare(backRank, 7);
            Square* rookTo = board.getSquare(backRank, 5);
            Piece* rook = rookFrom->getPiece();
            rookTo->setPiece(rook);
            rookFrom->clearPiece();
            // FIX: increment rook's move count so hasMoved() returns true
            rook->incrementMoveCount();
        }
        if (to->getCol() == 2) {   // queen-side
            Square* rookFrom = board.getSquare(backRank, 0);
            Square* rookTo = board.getSquare(backRank, 3);
            Piece* rook = rookFrom->getPiece();
            rookTo->setPiece(rook);
            rookFrom->clearPiece();
            // FIX: increment rook's move count
            rook->incrementMoveCount();
        }
    }

    // Handle en passant capture
    if (piece->getType() == PAWN && from->getCol() != to->getCol() && !isCapture) {
        Square* epSquare = board.getSquare(from->getRow(), to->getCol());
        if (epSquare && !epSquare->isEmpty()) {
            Piece* epPawn = epSquare->getPiece();
            Player& enemy = (currentTurn->getColor() == WHITE) ? black : white;
            Player& me = (currentTurn->getColor() == WHITE) ? white : black;
            enemy.removePiece(epPawn);
            me.recordCapture(epPawn);
            epSquare->clearPiece();
            isCapture = true;
            halfMoveClock = 0;   // en passant is a capture — reset clock
        }
    }

    // Reset en-passant flags for all current player's pawns, then mark double step
    Player& me = (currentTurn->getColor() == WHITE) ? white : black;
    for (int i = 0; i < me.getActiveCount(); i++) {
        Piece* p = me.getActivePiece(i);
        if (p->getType() == PAWN)
            static_cast<Pawn*>(p)->setDoubleStepTurn(-1);
    }
    if (piece->getType() == PAWN && abs(from->getRow() - to->getRow()) == 2)
        static_cast<Pawn*>(piece)->setDoubleStepTurn(turnNumber);

    // Handle promotion
    bool isPromotion = false;
    string promotionSuffix = "";
    if (piece->getType() == PAWN) {
        int promotionRank = (piece->getColor() == WHITE) ? 0 : 7;
        if (to->getRow() == promotionRank) {
            // FIX: build promotion suffix BEFORE calling handlePromotion
            //      (after it, 'piece' is no longer in the active list)
            switch (promotion) {
            case ROOK:   promotionSuffix = "=R"; break;
            case BISHOP: promotionSuffix = "=B"; break;
            case KNIGHT: promotionSuffix = "=N"; break;
            default:     promotionSuffix = "=Q"; break;
            }
            handlePromotion(to, piece->getColor(), promotion);
            isPromotion = true;
        }
    }

    updateStatus();

    // FIX: pass promotionSuffix to recordMove so notation is stored correctly
    history.recordMove(piece, from, to, isCapture,
        status == CHECK, status == CHECKMATE,
        promotionSuffix);

    switchTurn();
    return true;
}

void Game::run() {
    start();
    while (status == ONGOING || status == CHECK) {
        display();

        Square* from = nullptr;
        Square* to = nullptr;

        if (!input.getMove(board, from, to)) {
            status = RESIGNED;
            cout << "Game resigned.\n";
            break;
        }

        // Draw offer: from and to are both nullptr
        if (from == nullptr && to == nullptr) {
            Color  opponent = (currentTurn->getColor() == WHITE) ? BLACK : WHITE;
            string opponentName = (opponent == WHITE) ? "WHITE" : "BLACK";
            cout << opponentName << " do you accept the draw? (y/n): ";
            string response;
            getline(cin, response);
            size_t pos = response.find_first_not_of(" \t\r\n");
            char   ans = (pos != string::npos) ? tolower(response[pos]) : 'n';
            if (ans == 'y') {
                status = DRAW_AGREEMENT;
                display();
                cout << "Draw by agreement!\n";
            }
            else {
                cout << "Draw offer declined.\n";
            }
            continue;
        }

        Piece* movingPiece = from->getPiece();
        PieceType promotion = QUEEN;

        // FIX: only ask for promotion after basic legality checks pass
        if (movingPiece && movingPiece->getType() == PAWN) {
            int promotionRank = (movingPiece->getColor() == WHITE) ? 0 : 7;
            if (to->getRow() == promotionRank) {
                // Do a quick pre-check: is the destination even reachable?
                MoveList ml = movingPiece->availableMoves(board);
                if (ml.contains(to) && !wouldLeaveKingInCheck(from, to, movingPiece->getColor()))
                    promotion = input.getPromotion();
            }
        }

        if (!makeMove(from, to, promotion)) {
            cout << "Illegal move, try again.\n";
            continue;
        }

        if (status == CHECKMATE) {
            display();
            // currentTurn has already been switched — the winner is the other side
            cout << ((currentTurn->getColor() == WHITE) ? "BLACK" : "WHITE")
                << " wins by checkmate!\n";
            break;
        }
        if (status == STALEMATE) {
            display();
            cout << "Stalemate! It's a draw.\n";
            break;
        }
        if (status == DRAW_FIFTY_MOVE) {
            display();
            cout << "Draw by fifty move rule!\n";
            break;
        }
        if (status == DRAW_INSUFFICIENT) {
            display();
            cout << "Draw by insufficient material!\n";
            break;
        }
    }
}


// ═══════════════════════════════════════════════
//  INPUT HANDLER
// ═══════════════════════════════════════════════
// Returns: true  = valid input (move, draw offer, or bad-format retry)
//          false = user wants to quit
// Sets from=to=nullptr for a draw offer.
// Sets from=to=(Square*)-1 as a sentinel for bad-format (caller retries).
bool InputHandler::getMove(Board& board, Square*& from, Square*& to) {
    string line;
    cout << "Enter move (e.g. e2 e4), 'draw' to offer draw, or 'q' to quit: ";

    // Guard against EOF (e.g. piped input ending)
    if (!getline(cin, line)) return false;

    // Trim leading/trailing whitespace so "  e2 e4  " still works
    size_t start = line.find_first_not_of(" \t\r\n");
    size_t end = line.find_last_not_of(" \t\r\n");
    if (start == string::npos) {
        // Empty line — treat as bad format, not quit
        from = to = nullptr;
        cout << "No input entered, try again.\n";
        // Use sentinel: return true so caller loops, but leaves from/to null
        // We handle this by re-entering the function — just recurse once.
        return getMove(board, from, to);
    }
    line = line.substr(start, end - start + 1);

    if (wantsQuit(line))  return false;   // caller sets RESIGNED

    if (offersDraw(line)) {
        from = nullptr;
        to = nullptr;
        return true;
    }

    // Expect "XN XN" — at least 5 chars with a space in position 2
    if (line.size() < 5 || line[2] != ' ') {
        cout << "Bad format (use e.g. 'e2 e4'), try again.\n";
        return getMove(board, from, to);
    }

    string fromStr = line.substr(0, 2);
    string toStr = line.substr(3, 2);

    from = board.getSquare(fromStr);
    to = board.getSquare(toStr);

    if (!from || !to) {
        cout << "Square not recognised (use letters a-h and digits 1-8), try again.\n";
        return getMove(board, from, to);
    }

    return true;
}

PieceType InputHandler::getPromotion() {
    cout << "Promote to (Q = Queen, R = Rook, B = Bishop, N = Knight): ";
    while (true) {
        string line;
        // Use getline — consistent with the rest of the input system,
        // and correctly drains the whole line regardless of extra characters.
        if (!getline(cin, line)) return QUEEN;   // EOF fallback

        // Trim and grab first non-space character
        size_t pos = line.find_first_not_of(" \t\r\n");
        if (pos == string::npos) {
            cout << "No input, enter Q, R, B or N: ";
            continue;
        }
        char c = toupper(line[pos]);
        if (c == 'Q') return QUEEN;
        else if (c == 'R') return ROOK;
        else if (c == 'B') return BISHOP;
        else if (c == 'N') return KNIGHT;
        else cout << "Invalid, enter Q, R, B or N: ";
    }
}

bool InputHandler::wantsQuit(const string& input) {
    return input == "q" || input == "quit";
}

bool InputHandler::offersDraw(const string& input) {
    return input == "draw" || input == "d";
}


// ═══════════════════════════════════════════════
//  MOVE HISTORY
// ═══════════════════════════════════════════════
MoveHistory::MoveHistory() : moveCount(0) {}

// FIX: accepts promotionSuffix so promotion moves are stored correctly.
//      Also increased MAX_MOVES to 1000 in the header.
void MoveHistory::recordMove(Piece* piece, Square* from, Square* to,
    bool isCapture, bool isCheck, bool isCheckmate,
    const string& promotionSuffix) {
    if (moveCount >= MAX_MOVES) return;

    string notation = "";

    char symbol = piece->getSymbol();
    if (symbol != 'P' && symbol != 'p')
        notation += toupper(symbol);

    if (isCapture)  notation += "x";
    notation += to->getName();
    notation += promotionSuffix;    // FIX: append =Q / =R / =B / =N

    if (isCheckmate)      notation += "#";
    else if (isCheck)     notation += "+";

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