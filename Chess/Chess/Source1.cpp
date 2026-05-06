#include "ChessGUI.h"

int main() {
    Game    game;
    ChessGUI gui(game);
    gui.run();
    return 0;
}