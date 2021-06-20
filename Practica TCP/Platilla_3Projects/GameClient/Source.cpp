#pragma once

#include <PlayerInfo.h>
#include <SFML\Network.hpp>

#include <stdlib.h>
//#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
//#include <wait.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>


#define MAX 100
#define SIZE_TABLERO 64
#define SIZE_FILA_TABLERO 8
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

#define SIZE_TABLERO 64
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5


char tablero[SIZE_TABLERO];


/**
* Si guardamos las posiciones de las piezas con valores del 0 al 7,
* esta funciÃ³n las transforma a posiciÃ³n de ventana (pixel), que va del 0 al 512
*/
sf::Vector2f BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}

/**
* Contiene el cÃ³digo SFML que captura el evento del clic del mouse y el cÃ³digo que pinta por pantalla
*/

void DibujaSFML()
{
	
	sf::Vector2f casillaOrigen, casillaDestino;
	bool casillaMarcada = false;
	//bg
	sf::RectangleShape rectangle(sf::Vector2f(64, 64));
	rectangle.setFillColor(sf::Color(0, 250, 0));
	
	sf::Texture bg;

	if (!bg.loadFromFile("grass.png")) {
		std::cout << "Load failed!" << std::endl;
	}
	else {
		std::cout << "Load succes!" << std::endl;
	}

	//Tanque provisional
	sf::Texture tanqueUp;
	if (!tanqueUp.loadFromFile("tanque.png")) {
		std::cout << "Load failed!" << std::endl;
	}
	else {
		std::cout << "Load succes!" << std::endl;
	}

	sf::Texture b;
	if (!b.loadFromFile("bloque.png")) {
		std::cout << "Load failed!" << std::endl;
	}
	else {
		std::cout << "Load succes!" << std::endl;
	}
	/*sf::Texture tanqueLeft;
	if (!tanqueLeft.loadFromFile("yTank_Left.png")) {
		std::cout << "Load failed!" << std::endl;
	}
	else {
		std::cout << "Load succes!" << std::endl;
	}
	sf::Texture tanqueRight;
	if (!tanqueRight.loadFromFile("yTank_Right.png")) {
		std::cout << "Load failed!" << std::endl;
	}
	else {
		std::cout << "Load succes!" << std::endl;
	}
	sf::Texture tanqueDown;
	if (!tanqueDown.loadFromFile("yTank_Down.png")) {
		std::cout << "Load failed!" << std::endl;
	}
	else {
		std::cout << "Load succes!" << std::endl;
	}*/


	sf::Sprite bgSprite;
	bgSprite.setTexture(bg);
	
	sf::Sprite tankSp_up;
	tankSp_up.setTexture(tanqueUp);
	tankSp_up.scale(5, 5);
	
	sf::Sprite bloque;
	bloque.setTexture(b);
	bloque.scale(5, 5);


	/*sf::Sprite tankSp_left;
	tankSp_left.setTexture(tanqueLeft);
	tankSp_left.scale(5, 5);

	sf::Sprite tankSp_down;
	tankSp_down.setTexture(tanqueRight);
	tankSp_down.scale(5, 5);

	sf::Sprite tankSp_right;
	tankSp_right.setTexture(tanqueDown);
	tankSp_right.scale(5, 5);
	*/
	//sprite.scale(0.60, 0.60);
	//sprite.setTextureRect(sf::IntRect(0, 0, 512, 512));
	sf::RenderWindow window(sf::VideoMode(512, 512), "Ejemplo tablero");
	while (window.isOpen())
	{
		sf::Event event;
		//Este primer WHILE es para controlar los eventos del mouse
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			default:
				break;
			}
		}
		window.clear();
		window.draw(bgSprite);
		
		window.draw(rectangle);
		window.draw(tankSp_up);

		if (event.type == sf::Event::KeyPressed) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
				sf::Vector2f A = tankSp_up.getPosition();
				std::cout << A.x << std::endl;
				std::cout << A.y << std::endl;
				tankSp_up.move(0, -0.05);

			}
		}

		if (event.type == sf::Event::KeyPressed) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
				
				tankSp_up.move(0, 0.05);
				
			}
		}

		if (event.type == sf::Event::KeyPressed) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
				
				
				tankSp_up.move(-0.05, 0);
				
			}
		}

		if (event.type == sf::Event::KeyPressed) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				
				tankSp_up.move(0.05, 0);
			}
		}
		






		//A partir de aquÃ­ es para pintar por pantalla
		//Este FOR es para el tablero
		//window.draw(sprite);

		for (int i = 0; i<8; i++)
		{
			for (int j = 0; j<8; j++)
			{
				//sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));
				//rectBlanco.setFillColor(sf::Color::Black);
				if (i % 5 == 0)
				{
					//Empieza por el blanco
					if (j % 6 == 0)
					{
						bloque.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
						window.draw(bloque);
					}
				}
				else
				{
					//Empieza por el negro
					if (j % 2 == 1)
					{
						bloque.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
						window.draw(bloque);
					}
				}
			}
		}
		//Para pintar el un circulito
		//sf::CircleShape shape(RADIO_AVATAR);
		//shape.setFillColor(sf::Color::Red);
		//sf::Vector2f posicion_bolita(4.f, 7.f);
		//posicion_bolita = BoardToWindows(posicion_bolita);
		//shape.setPosition(posicion_bolita);
		//window.draw(shape);
		window.display();
	}

}

int main()
{
	DibujaSFML();
	PlayerInfo playerInfo;
	return 0;
}