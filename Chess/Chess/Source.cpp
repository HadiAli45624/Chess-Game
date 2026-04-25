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