#include "Chessgui.h"
#include <sstream>
#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;

// ═══════════════════════════════════════════════
//  CONSTRUCTION
// ═══════════════════════════════════════════════
ChessGUI::ChessGUI(Game& game)
    : game(game),
    window(sf::VideoMode({ (unsigned int)WINDOW_WIDTH, (unsigned int)WINDOW_HEIGHT }), "Chess",
        sf::Style::Titlebar | sf::Style::Close),
    selectedSquare(nullptr),
    lastFrom(nullptr),
    lastTo(nullptr),
    promotionState(PromotionState::NONE),
    promotionFrom(nullptr),
    promotionTo(nullptr)
{
    window.setFramerateLimit(60);

    bool loaded = font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!loaded) loaded = font.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!loaded) loaded = font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    if (!loaded)
        std::cerr << "Warning: could not load font — text will not render.\n";

    loadTextures();
    legalMoves = MoveList();
    fontLoaded = loaded;
}


// ═══════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════
void ChessGUI::run() {
    game.start();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            handleEvent(*event);
        }

        render();
        window.display();
    }
}


// ═══════════════════════════════════════════════
//  EVENT HANDLING  (SFML 3 variant-based events)
// ═══════════════════════════════════════════════
void ChessGUI::handleEvent(const sf::Event& event) {
    // SFML 3: use std::holds_alternative / event.getIf<T>()
    if (event.is<sf::Event::Closed>()) {
        window.close();
        return;
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            int px = mb->position.x;
            int py = mb->position.y;

            if (promotionState == PromotionState::WAITING) {
                handlePromotionClick(px, py);
            }
            else {
                if (px < BOARD_SIZE)
                    handleBoardClick(px, py);
                else
                    handleSidebarClick(px, py);
            }
        }
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::R) {
            clearSelection();
            game.reset();
        }
        if (kp->code == sf::Keyboard::Key::Escape)
            clearSelection();
    }
}

void ChessGUI::handleBoardClick(int pixelX, int pixelY) {
    State s = game.getStatus();
    if (s == CHECKMATE || s == STALEMATE || s == DRAW_FIFTY_MOVE ||
        s == DRAW_INSUFFICIENT || s == DRAW_AGREEMENT || s == RESIGNED)
        return;

    int row, col;
    if (!pixelToBoard(pixelX, pixelY, row, col)) return;

    Board& board = game.getBoard();
    Square* clicked = board.getSquare(row, col);
    if (!clicked) return;

    if (!selectedSquare) {
        if (clicked->isEmpty()) return;
        if (clicked->getPiece()->getColor() != game.getCurrentTurnColor()) return;
        selectedSquare = clicked;
        legalMoves = game.getLegalMovesFrom(clicked);
        return;
    }

    if (clicked == selectedSquare) {
        clearSelection();
        return;
    }

    if (!clicked->isEmpty() &&
        clicked->getPiece()->getColor() == game.getCurrentTurnColor())
    {
        selectedSquare = clicked;
        legalMoves = game.getLegalMovesFrom(clicked);
        return;
    }

    if (!legalMoves.contains(clicked)) {
        clearSelection();
        return;
    }

    Piece* movingPiece = selectedSquare->getPiece();
    if (movingPiece && movingPiece->getType() == PAWN) {
        int promotionRank = (movingPiece->getColor() == WHITE) ? 0 : 7;
        if (clicked->getRow() == promotionRank) {
            promotionFrom = selectedSquare;
            promotionTo = clicked;
            promotionState = PromotionState::WAITING;
            clearSelection();
            return;
        }
    }

    Square* from = selectedSquare;
    clearSelection();

    if (game.makeMove(from, clicked)) {
        lastFrom = from;
        lastTo = clicked;
    }
}

void ChessGUI::handlePromotionClick(int pixelX, int pixelY) {
    const int btnW = 100;
    const int btnH = 100;
    const int totalW = 4 * btnW;
    const int startX = (BOARD_SIZE - totalW) / 2;
    const int startY = (BOARD_SIZE - btnH) / 2;

    if (pixelY < startY || pixelY > startY + btnH) {
        promotionState = PromotionState::NONE;
        promotionFrom = nullptr;
        promotionTo = nullptr;
        return;
    }

    int slot = (pixelX - startX) / btnW;
    if (slot < 0 || slot > 3) {
        promotionState = PromotionState::NONE;
        return;
    }

    PieceType choices[4] = { QUEEN, ROOK, BISHOP, KNIGHT };
    PieceType chosen = choices[slot];

    promotionState = PromotionState::NONE;
    Square* from = promotionFrom;
    Square* to = promotionTo;
    promotionFrom = nullptr;
    promotionTo = nullptr;

    if (game.makeMove(from, to, chosen)) {
        lastFrom = from;
        lastTo = to;
    }
}

void ChessGUI::handleSidebarClick(int pixelX, int pixelY) {
    // Existing New Game button
    const int btnX = BOARD_SIZE + 20;
    const int btnW = SIDEBAR_WIDTH - 40;

    // New Game
    const int newGameY = WINDOW_HEIGHT - 70;
    if (pixelX >= btnX && pixelX <= btnX + btnW &&
        pixelY >= newGameY && pixelY <= newGameY + 40)
    {
        clearSelection();
        game.reset();
        lastFrom = nullptr;
        lastTo = nullptr;
    }

    // Resign
    const int resignY = WINDOW_HEIGHT - 130;
    if (pixelX >= btnX && pixelX <= btnX + btnW &&
        pixelY >= resignY && pixelY <= resignY + 40)
    {
        game.resign();  // you need to implement this in Game
    }

    // Offer Draw
    const int drawY = WINDOW_HEIGHT - 190;
    if (pixelX >= btnX && pixelX <= btnX + btnW &&
        pixelY >= drawY && pixelY <= drawY + 40)
    {
        game.offersDraw();  // you need to implement this in Game
    }
}


// ═══════════════════════════════════════════════
//  RENDERING
// ═══════════════════════════════════════════════
void ChessGUI::render() {
    window.clear(COLOR_SIDEBAR_BG);
    drawBoard();
    drawHighlights();
    drawPieces();
    drawCoordinates();
    drawSidebar();

    if (promotionState == PromotionState::WAITING)
        drawPromotionOverlay();
}

void ChessGUI::drawBoard() {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            sf::RectangleShape sq(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
            sq.setPosition(sf::Vector2f(c * SQUARE_SIZE, r * SQUARE_SIZE));
            sq.setFillColor(((r + c) % 2 == 0) ? COLOR_LIGHT : COLOR_DARK);
            window.draw(sq);
        }
    }
}

void ChessGUI::drawHighlights() {
    Board& board = game.getBoard();

    auto drawOverlay = [&](Square* sq, sf::Color color) {
        if (!sq) return;
        sf::RectangleShape hl(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
        hl.setPosition(sf::Vector2f(sq->getCol() * SQUARE_SIZE, sq->getRow() * SQUARE_SIZE));
        hl.setFillColor(color);
        window.draw(hl);
        };

    drawOverlay(lastFrom, COLOR_LAST_FROM);
    drawOverlay(lastTo, COLOR_LAST_TO);

    Color turn = game.getCurrentTurnColor();
    if (game.getStatus() == CHECK || game.getStatus() == CHECKMATE) {
        Player& me = (turn == WHITE) ? game.getWhitePlayer() : game.getBlackPlayer();
        for (int i = 0; i < me.getActiveCount(); i++) {
            Piece* p = me.getActivePiece(i);
            if (p->getType() == KING) {
                drawOverlay(p->getSquare(), COLOR_CHECK);
                break;
            }
        }
    }

    drawOverlay(selectedSquare, COLOR_SELECTED);

    for (int i = 0; i < legalMoves.count; i++) {
        Square* sq = legalMoves.get(i);
        if (!sq) continue;

        float px = static_cast<float>(sq->getCol() * SQUARE_SIZE);
        float py = static_cast<float>(sq->getRow() * SQUARE_SIZE);

        if (sq->isEmpty()) {
            sf::CircleShape dot(SQUARE_SIZE * 0.15f);
            dot.setFillColor(COLOR_LEGAL);
            dot.setOrigin(sf::Vector2f(dot.getRadius(), dot.getRadius()));
            dot.setPosition(sf::Vector2f(px + SQUARE_SIZE / 2.f, py + SQUARE_SIZE / 2.f));
            window.draw(dot);
        }
        else {
            sf::CircleShape ring(SQUARE_SIZE * 0.46f);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(5.f);
            ring.setOutlineColor(COLOR_LEGAL);
            ring.setOrigin(sf::Vector2f(ring.getRadius(), ring.getRadius()));
            ring.setPosition(sf::Vector2f(px + SQUARE_SIZE / 2.f, py + SQUARE_SIZE / 2.f));
            window.draw(ring);
        }
    }
}

void ChessGUI::drawPieces() {
    Board& board = game.getBoard();
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Square* sq = board.getSquare(r, c);
            if (!sq->isEmpty())
                drawPiece(sq->getPiece(), c, r);
        }
    }
}

// ── Helper: create a configured sf::Text (SFML 3 requires font at construction) ──
//sf::Text ChessGUI::makeText(const string& str, unsigned int size, sf::Color color) const {
//    sf::Text t(font, sf::String::fromUtf8(str.begin(), str.end()), size);
//    t.setFillColor(color);
//    return t;
//}

void ChessGUI::drawPiece(Piece* piece, int col, int row) {
    string key = "";
    key += (piece->getColor() == WHITE) ? 'w' : 'b';
    switch (piece->getType()) {
    case PAWN:   key += 'P'; break;
    case ROOK:   key += 'R'; break;
    case KNIGHT: key += 'N'; break;
    case BISHOP: key += 'B'; break;
    case QUEEN:  key += 'Q'; break;
    case KING:   key += 'K'; break;
    }

    auto it = pieceTextures.find(key);
    if (it == pieceTextures.end()) return;

    sf::Sprite sprite(it->second);

    // Scale sprite to fit square
    sf::Vector2u texSize = it->second.getSize();
    float scaleX = (SQUARE_SIZE * PIECE_SCALE) / texSize.x;
    float scaleY = (SQUARE_SIZE * PIECE_SCALE) / texSize.y;
    sprite.setScale(sf::Vector2f(scaleX, scaleY));

    // Centre in square
    float offsetX = (SQUARE_SIZE - texSize.x * scaleX) / 2.f;
    float offsetY = (SQUARE_SIZE - texSize.y * scaleY) / 2.f;
    sprite.setPosition(sf::Vector2f(col * SQUARE_SIZE + offsetX, row * SQUARE_SIZE + offsetY));

    window.draw(sprite);
}

void ChessGUI::drawCoordinates() {
    if (!fontLoaded) return;

    for (int c = 0; c < 8; c++) {
        sf::Text t(font, string(1, 'a' + c), 12);
        t.setFillColor(((c % 2) == 0) ? COLOR_DARK : COLOR_LIGHT);
        t.setPosition(sf::Vector2f(c * SQUARE_SIZE + SQUARE_SIZE - 14.f, BOARD_SIZE - 16.f));
        window.draw(t);
    }
    for (int r = 0; r < 8; r++) {
        sf::Text t(font, to_string(8 - r), 12);
        t.setFillColor(((r % 2) == 0) ? COLOR_DARK : COLOR_LIGHT);
        t.setPosition(sf::Vector2f(3.f, r * SQUARE_SIZE + 3.f));
        window.draw(t);
    }
}

void ChessGUI::drawSidebar() {
    // Background FIRST so it doesn't cover buttons
    sf::RectangleShape bg(sf::Vector2f(SIDEBAR_WIDTH, WINDOW_HEIGHT));
    bg.setPosition(sf::Vector2f(BOARD_SIZE, 0));
    bg.setFillColor(COLOR_SIDEBAR_BG);
    window.draw(bg);

    sf::RectangleShape sep(sf::Vector2f(2, WINDOW_HEIGHT));
    sep.setPosition(sf::Vector2f(BOARD_SIZE, 0));
    sep.setFillColor(sf::Color(60, 60, 60));
    window.draw(sep);

    if (!fontLoaded) return;

    float x = BOARD_SIZE + 16.f;
    float y = 16.f;
    float lineH = 22.f;

    auto label = [&](const string& str, sf::Color color, unsigned int size = 15) {
        sf::Text t(font, str, size);
        t.setFillColor(color);
        t.setPosition(sf::Vector2f(x, y));
        window.draw(t);
        y += lineH;
        };

    label("CHESS", COLOR_TEXT_MAIN, 22);
    y += 6;

    State s = game.getStatus();
    label(statusMessage(), s == CHECK ? sf::Color(255, 120, 120) : COLOR_TEXT_MAIN, 14);
    y += 4;

    if (s == ONGOING || s == CHECK) {
        string turnStr = (game.getCurrentTurnColor() == WHITE) ? "White to move" : "Black to move";
        label(turnStr, COLOR_TEXT_DIM, 13);
    }
    y += 10;

    sf::RectangleShape hline(sf::Vector2f(SIDEBAR_WIDTH - 32, 1));
    hline.setPosition(sf::Vector2f(x, y));
    hline.setFillColor(sf::Color(60, 60, 60));
    window.draw(hline);
    y += 10;

    label("Captured", COLOR_TEXT_DIM, 12);

    auto drawCaps = [&](Player& player, sf::Color capColor) {
        string caps = "";
        for (int i = 0; i < player.getCapturedCount(); i++) {
            Piece* p = player.getCapturedPiece(i);
            caps += pieceGlyph(p->getType(), p->getColor());
        }
        if (!caps.empty()) {
            sf::Text t(font, sf::String::fromUtf8(caps.begin(), caps.end()), 18);
            t.setFillColor(capColor);
            t.setPosition(sf::Vector2f(x, y));
            window.draw(t);
        }
        y += lineH + 4;
        };

    drawCaps(game.getWhitePlayer(), COLOR_TEXT_MAIN);
    drawCaps(game.getBlackPlayer(), COLOR_TEXT_DIM);
    y += 12;

    sf::RectangleShape hline2(sf::Vector2f(SIDEBAR_WIDTH - 32, 1));
    hline2.setPosition(sf::Vector2f(x, y));
    hline2.setFillColor(sf::Color(60, 60, 60));
    window.draw(hline2);
    y += 10;

    // All buttons at the bottom — declare btnX and btnW once here
    const int btnX = BOARD_SIZE + 20;
    const int btnW = SIDEBAR_WIDTH - 40;

    // Offer Draw button
    const int drawBtnY = WINDOW_HEIGHT - 190;
    sf::RectangleShape drawBtn(sf::Vector2f((float)btnW, 40));
    drawBtn.setPosition(sf::Vector2f((float)btnX, (float)drawBtnY));
    drawBtn.setFillColor(sf::Color(70, 100, 140));
    drawBtn.setOutlineColor(sf::Color(100, 140, 180));
    drawBtn.setOutlineThickness(1.f);
    window.draw(drawBtn);

    sf::Text drawText(font, "Offer Draw", 15);
    drawText.setFillColor(sf::Color::White);
    sf::FloatRect dt = drawText.getLocalBounds();
    drawText.setPosition(sf::Vector2f(
        btnX + (btnW - dt.size.x) / 2.f - dt.position.x,
        drawBtnY + (40 - dt.size.y) / 2.f - dt.position.y));
    window.draw(drawText);

    // Resign button
    const int resignBtnY = WINDOW_HEIGHT - 130;
    sf::RectangleShape resignBtn(sf::Vector2f((float)btnW, 40));
    resignBtn.setPosition(sf::Vector2f((float)btnX, (float)resignBtnY));
    resignBtn.setFillColor(sf::Color(140, 60, 60));
    resignBtn.setOutlineColor(sf::Color(180, 100, 100));
    resignBtn.setOutlineThickness(1.f);
    window.draw(resignBtn);

    sf::Text resignText(font, "Resign", 15);
    resignText.setFillColor(sf::Color::White);
    sf::FloatRect rt = resignText.getLocalBounds();
    resignText.setPosition(sf::Vector2f(
        btnX + (btnW - rt.size.x) / 2.f - rt.position.x,
        resignBtnY + (40 - rt.size.y) / 2.f - rt.position.y));
    window.draw(resignText);

    // New Game button
    const int btnY = WINDOW_HEIGHT - 70;
    sf::RectangleShape btn(sf::Vector2f((float)btnW, 40));
    btn.setPosition(sf::Vector2f((float)btnX, (float)btnY));
    btn.setFillColor(sf::Color(60, 110, 70));
    btn.setOutlineColor(sf::Color(100, 160, 110));
    btn.setOutlineThickness(1.f);
    window.draw(btn);

    sf::Text btnText(font, "New Game (R)", 15);
    btnText.setFillColor(sf::Color::White);
    sf::FloatRect tb = btnText.getLocalBounds();
    btnText.setPosition(sf::Vector2f(
        btnX + (btnW - tb.size.x) / 2.f - tb.position.x,
        btnY + (40 - tb.size.y) / 2.f - tb.position.y));
    window.draw(btnText);
}

void ChessGUI::drawPromotionOverlay() {
    sf::RectangleShape dim(sf::Vector2f(BOARD_SIZE, BOARD_SIZE));
    dim.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dim);

    const int btnW = 100;
    const int btnH = 100;
    const int totalW = 4 * btnW;
    const int startX = (BOARD_SIZE - totalW) / 2;
    const int startY = (BOARD_SIZE - btnH) / 2;

    PieceType options[4] = { QUEEN, ROOK, BISHOP, KNIGHT };
    Color promColor = (promotionTo && promotionTo->getRow() == 0) ? WHITE : BLACK;

    for (int i = 0; i < 4; i++) {
        float bx = static_cast<float>(startX + i * btnW);
        float by = static_cast<float>(startY);

        sf::RectangleShape box(sf::Vector2f(btnW - 4, btnH - 4));
        box.setPosition(sf::Vector2f(bx + 2, by + 2));
        box.setFillColor(sf::Color(50, 50, 50));
        box.setOutlineColor(sf::Color(180, 180, 180));
        box.setOutlineThickness(2.f);
        window.draw(box);

        string glyph = pieceGlyph(options[i], promColor);
        sf::Text t(font, sf::String::fromUtf8(glyph.begin(), glyph.end()),
            static_cast<unsigned int>(btnH * 0.65f));
        t.setFillColor(promColor == WHITE ? sf::Color::White : sf::Color(30, 30, 30));
        t.setOutlineColor(promColor == WHITE ? sf::Color(60, 60, 60) : sf::Color(200, 200, 200));
        t.setOutlineThickness(1.5f);

        sf::FloatRect bounds = t.getLocalBounds();
        t.setPosition(sf::Vector2f(
            bx + (btnW - bounds.size.x) / 2.f - bounds.position.x,
            by + (btnH - bounds.size.y) / 2.f - bounds.position.y));
        window.draw(t);
    }

    if (fontLoaded) {
        sf::Text lbl(font, "Promote to:", 16);
        lbl.setFillColor(sf::Color::White);
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setPosition(sf::Vector2f(
            (BOARD_SIZE - lb.size.x) / 2.f - lb.position.x,
            static_cast<float>(startY - 30)));
        window.draw(lbl);
    }
}


// ═══════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════
sf::Vector2i ChessGUI::boardToPixel(int row, int col) const {
    return { col * SQUARE_SIZE, row * SQUARE_SIZE };
}

bool ChessGUI::pixelToBoard(int px, int py, int& row, int& col) const {
    col = px / SQUARE_SIZE;
    row = py / SQUARE_SIZE;
    return (col >= 0 && col < 8 && row >= 0 && row < 8);
}

string ChessGUI::pieceGlyph(PieceType type, Color color) const {
    static const char* white_glyphs[] = {
        "\xe2\x99\x99", "\xe2\x99\x96", "\xe2\x99\x98",
        "\xe2\x99\x97", "\xe2\x99\x95", "\xe2\x99\x94",
    };
    static const char* black_glyphs[] = {
        "\xe2\x99\x9f", "\xe2\x99\x9c", "\xe2\x99\x9e",
        "\xe2\x99\x9d", "\xe2\x99\x9b", "\xe2\x99\x9a",
    };
    return (color == WHITE) ? white_glyphs[type] : black_glyphs[type];
}

string ChessGUI::statusMessage() const {
    switch (game.getStatus()) {
    case ONGOING:           return "Game in progress";
    case CHECK:             return "Check!";
    case CHECKMATE:         return "Checkmate!";
    case STALEMATE:         return "Stalemate — Draw";
    case DRAW_FIFTY_MOVE:   return "Draw — 50 move rule";
    case DRAW_INSUFFICIENT: return "Draw — insufficient material";
    case DRAW_AGREEMENT:    return "Draw by agreement";
    case RESIGNED:          return "Game resigned";
    default:                return "";
    }
}

void ChessGUI::clearSelection() {
    selectedSquare = nullptr;
    legalMoves = MoveList();
}

void ChessGUI::loadTextures() {
    pieceTextures["wP"].loadFromFile("pieces/wP.png");
    pieceTextures["wR"].loadFromFile("pieces/wR.png");
    pieceTextures["wN"].loadFromFile("pieces/wN.png");
    pieceTextures["wB"].loadFromFile("pieces/wB.png");
    pieceTextures["wQ"].loadFromFile("pieces/wQ.png");
    pieceTextures["wK"].loadFromFile("pieces/wK.png");
    pieceTextures["bP"].loadFromFile("pieces/bP.png");
    pieceTextures["bR"].loadFromFile("pieces/bR.png");
    pieceTextures["bN"].loadFromFile("pieces/bN.png");
    pieceTextures["bB"].loadFromFile("pieces/bB.png");
    pieceTextures["bQ"].loadFromFile("pieces/bQ.png");
    pieceTextures["bK"].loadFromFile("pieces/bK.png");
}
