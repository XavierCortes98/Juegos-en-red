#include "Map.h"
#include <vector>
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>

Map::Map()
{
}


Map::~Map()
{
}

void Map::printChar(int x, int y)
{
	std::cout << "El char de esta casilla es: " << map[x][y] << std::endl;
}

void Map::printMap()
{
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++) {
			std::cout << map[i][j] << " ";
		}
		std::cout << std::endl;
	}

}

bool Map::validMovement(int x, int y)
{
	if (map[x][y] == '-' || map[x][y] == 'o')
	{
		return true;
	}
	else {
		return false;
	}

}

void Map::updatemap(int x, int y, char a)
{
	map[x][y] = a;
}

void Map::setRandomFood()
{
	
	int v1,v2;

	for (int i = 0; i < 40; i++)
	{
		v1 = rand() % 10 + 1;
		v2 = rand() % 10 + 1;

		map[v1][v2] = 'a';


	}
	
}
