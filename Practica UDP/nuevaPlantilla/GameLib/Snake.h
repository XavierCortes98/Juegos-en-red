#pragma once
#include <SFML\Graphics.hpp>
#include <iostream>

class Snake
{
public:
	Snake();
	~Snake();

	sf::Texture texture_;
	sf::Sprite sprite_;
	

	void resetSnakeSize();
	void setSnakeSize();
	int getSnakeSize();

	int snakeID;
	
private:
	int snakeSize;
};

