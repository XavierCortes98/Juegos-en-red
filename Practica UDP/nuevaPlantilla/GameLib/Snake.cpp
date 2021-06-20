#include "Snake.h"



Snake::Snake()
{
}


Snake::~Snake()
{
}


void Snake::resetSnakeSize()
{
	snakeSize == 0;
}

void Snake::setSnakeSize()
{
	snakeSize++;
}

int Snake::getSnakeSize()
{
	return snakeSize;
}
