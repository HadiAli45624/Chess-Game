#pragma once

#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>
#include "Header.h"
#include <map>
#include <string>

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────
static const int   SQUARE_SIZE = 80;           // px per board square
static const int   BOARD_SIZE = SQUARE_SIZE * 8;
static const int   SIDEBAR_WIDTH = 240;
static const int   WINDOW_WIDTH = BOARD_SIZE + SIDEBAR_WIDTH;
static const int   WINDOW_HEIGHT = BOARD_SIZE;
static const float PIECE_SCALE = 0.72f;        // piece glyph relative to square

// Board colours
static const sf::Color COLOR_LIGHT{ 240, 217, 181 };   // cream
static const sf::Color COLOR_DARK{ 81, 130,  90 };   // green
static const sf::Color COLOR_SELECTED{ 246, 246, 105, 180 };
static const sf::Color COLOR_LEGAL{ 106, 187,  87, 160 };
static const sf::Color COLOR_LAST_FROM{ 205, 210,  85, 150 };
static const sf::Color COLOR_LAST_TO{ 205, 210,  85, 150 };
static const sf::Color COLOR_CHECK{ 220,  50,  50, 180 };
static const sf::Color COLOR_SIDEBAR_BG{ 34,  34,  34 };
static const sf::Color COLOR_TEXT_MAIN{ 230, 230, 230 };
static const sf::Color COLOR_TEXT_DIM{ 140, 140, 140 };

// ─────────────────────────────────────────────
//  GUI STATE
// ─────────────────────────────────────────────
enum class PromotionState { NONE, WAITING };

// ─────────────────────────────────────────────
//  CHESSGUI
// ─────────────────────────────────────────────
class ChessGUI {
public:
    ChessGUI(Game& game);

    // Enter the SFML event/render loop — returns when window is closed
    void run();

private:
    // ── Core references ─────────────────────
    Game& game;
    sf::RenderWindow  window;
    sf::Font          font;
    bool              fontLoaded = false;

    // ── Selection state ─────────────────────
    Square* selectedSquare;    // first click
    MoveList          legalMoves;        // moves for selectedSquare
    Square* lastFrom;          // last move highlight
    Square* lastTo;

    // ── Promotion state ──────────────────────
    PromotionState    promotionState;
    Square* promotionFrom;
    Square* promotionTo;

    // ── Event handling ───────────────────────
    void handleEvent(const sf::Event& event);
    void handleBoardClick(int pixelX, int pixelY);
    void handlePromotionClick(int pixelX, int pixelY);
    void handleSidebarClick(int pixelX, int pixelY);

    // ── Rendering ────────────────────────────
    void render();
    void drawBoard();
    void drawHighlights();
    void drawPieces();
    void drawPiece(Piece* piece, int col, int row);
    void drawSidebar();
    void drawPromotionOverlay();
    void drawCoordinates();

    // ── Helpers ──────────────────────────────
    sf::Vector2i boardToPixel(int row, int col) const;
    bool         pixelToBoard(int px, int py, int& row, int& col) const;
    string       pieceGlyph(PieceType type, Color color) const;
    string       statusMessage() const;
    void         clearSelection();

    std::map<std::string, sf::Texture> pieceTextures;
    void loadTextures();
};