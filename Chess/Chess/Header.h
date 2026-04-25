#pragma once
#include <iostream>
using namespace std;

enum State {ONGOING, CHECK, CHECKMATE, STALEMATE, DRAW};
enum Color {WHITE, BLACK};

class Game {
	Board board;
	Player white;
	Player black;
	Player* currentTurn = &white;
	State status = ONGOING;

};

class Board {
	Square squares[8][8];
public:
	Board();
};

class Square {
	std::string name;
	Piece* piece;
public:
	Square();
	Square(string name);
	
};

class Piece {
	Square* square;
	Color color;
public:

};


class Player {
	Color color;
	Piece* activePieces[16];
	Piece* capturedPieces[16];
	int activeCount;
	int capturedCount;
};