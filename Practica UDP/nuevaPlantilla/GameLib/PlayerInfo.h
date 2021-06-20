#pragma once
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>

enum msgType { HELLO, WELCOME, ACKNWELLCOME, NOTWELCOME, NEWPLAYER, ACKN, MOVEMENT, OKMOVEMENT, BADMOVEMENT, NOMOVEMENT, PING, PINGACKN, COLLISION, ENDGAME, UPDATEGAME, ENDGAMEDISCONECT};
struct clientProxy
{
	int pwpPlus;
	int pwpMinus;
	int victory;
	int defeat;
	sf::Vector2i pos;
	sf::IpAddress ip;
	unsigned short port;
	std::string alias;
	int numPings;
	int idPlayer;
	int points;
};

struct players
{
	int victory;
	sf::Vector2i pos;
	std::string alias;
	int playerid;
	int points;
};

class PlayerInfo
{
	std::string name;
	sf::Vector2i position;
	int lives;
public:
	PlayerInfo();
	~PlayerInfo();
};

struct AccumMovement {
	int idMove;
	int AbsoluteX;
	int AbsoluteY;
};

struct AccumMovementServer {
	int idPlayer;
	int idMove;
	int AbsoluteX;
	int AbsoluteY;
};
float lerp(int oldPosValue, int newPosValue, int t) {

	return (1 - t)*oldPosValue + t*newPosValue;
}

struct rocasCliente {
	int idRock;
	sf::RectangleShape roca;
	int posX;
	int posY;
};

struct rocasServidor {
	int idRock;
	int color;
	int posX;
	int posY;
};

std::vector<rocasCliente> obstaculosServidor;
std::vector<rocasServidor> obstaculosServidorAux;
std::vector<rocasCliente> obstaculosCliente;
int t_ = 0;