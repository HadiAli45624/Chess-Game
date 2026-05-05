#include "Header.h"

//Board Functions
Board::Board()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			char file = 'a' + j;
			char rank = '8' - i;
			string name = "";
			name += file;
			name += rank;
			squares[i][j] = Square(name);
		}
	}
}



//Square Functions

Square::Square() {
	name = "";
	piece = nullptr;
}

Square::Square(string name)
{
	this->name = name;
	piece = nullptr;
}

//MoveHistoryFunctions

MoveHistory::MoveHistory() {
	moveCount = 0;
}

void MoveHistory::recordMove(string move) {
	if (moveCount < 500) {
		moves[moveCount] = move;
		moveCount++;
	}
}

void MoveHistory::printHistory() {
	for (int i = 0; i < moveCount; i++) {
		cout << i + 1 << ". " << moves[i] << endl;
	}
}

string MoveHistory::getLastMove() {
	if (moveCount == 0) return "";
	return moves[moveCount - 1];
}

void MoveHistory::undoLast() {
	if (moveCount > 0) {
		moves[moveCount - 1] = "";
		moveCount--;
	}
}