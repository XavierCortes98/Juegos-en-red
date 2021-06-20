#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <thread>
#include <mutex>
#include <Windows.h>
#include <PlayerInfo.h>
#include <PowerUps.h>
#include <map>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>


#define SIZE_TABLERO 144
#define SIZE_FILA_TABLERO 12
#define LADO_CASILLA 144
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5
#define PINGTIME 1000
#define	UPDATETIME 2000
#define PCKTIME 2000
#define PROB 10

#define HOST "tcp://www.db4free.net:3306"
#define USER "redescarmen"
#define PASSWORD "chistorra"
#define DATABASE "bdpracticaudp"

std::vector<AccumMovementServer>accums;

std::mutex mtx;

void losePacket(sf::Packet &_pck)
{
	int prob = rand() % 100;

	if (prob < PROB)
	{
		_pck.clear();
	}
}

void closeSesion(clientProxy client, sql::Driver* driver, sql::Connection* con);

std::map<int, std::vector<AccumMovementServer>> mapMovements;
bool deadRocks;

void validateMovement(sf::UdpSocket* sock, std::vector<clientProxy> &clients)
{
	sf::Packet pckToSend;
	//recorremos los mensajes
	while (true)
	{
		mtx.lock();
		for (auto &map : mapMovements)
		{
			for (int i = 0; i < map.second.size(); i++)
			{
				//validamos el movimiento
				if (map.second[i].AbsoluteX < 0 || map.second[i].AbsoluteX >= 504 || map.second[i].AbsoluteY < 0 || map.second[i].AbsoluteY >= 64)
				{
					//En el caso de que no sea válido el movimiento
					for (int j = 0; j < clients.size(); j++)
					{
						if (clients[j].idPlayer == map.second[i].idPlayer) //Es al que le enviamos la validación
						{
							int auxId = map.second[i].idMove;
							//std::cout << "El movimiento " << map.second[i].AbsoluteX << " NO es valido" << std::endl;
							pckToSend << BADMOVEMENT << auxId;
							sock->send(pckToSend, clients[j].ip, clients[j].port);
							pckToSend.clear();
							break;
						}
					}
				}
				else if (i == map.second.size() - 1)
				{
					//En el caso de que sea válido el movimiento
					for (int j = 0; j < clients.size(); j++)
					{

						if (clients[j].idPlayer == map.second[i].idPlayer) //Es al que le enviamos la validación
						{
							clients[j].pos.x = map.second[i].AbsoluteX;
							//std::cout << "El movimiento "<< map.second[i].AbsoluteX << " es valido"<< std::endl;
							pckToSend << OKMOVEMENT << map.second[i].idMove;
							sock->send(pckToSend, clients[j].ip, clients[j].port);
							pckToSend.clear();
						}
						else { //Le enviamos el movimiento al rival
							int x, y;
							x = map.second[i].AbsoluteX;
							pckToSend << MOVEMENT << x;
							sock->send(pckToSend, clients[j].ip, clients[j].port);
							pckToSend.clear();
						}
					}
				}
			}
		}
		mapMovements.clear(); //reseteamos el mapa una vez validados los movimientos
		mtx.unlock();
		Sleep(1000);
	}
}

bool finDeljuego;

void disconnectPlayer(sf::UdpSocket* sock, int a, std::vector<clientProxy> &clients, sql::Driver* driver, sql::Connection* con)
{	
	sf::Packet pck;
	closeSesion(clients[a], driver, con);
	clients.erase(clients.begin() + a);
	finDeljuego = true;
	if (a = 0) { //El que se ha desconectado
		pck << ENDGAMEDISCONECT;
		if (!sock->send(pck, clients[1].ip, clients[1].port))
		{
			std::cout << "errrrrrrrrrrrrrrror" << std::endl;
		}
		pck.clear();
	}
	else {
		pck << ENDGAMEDISCONECT;
		if (!sock->send(pck, clients[0].ip, clients[0].port))
		{
			std::cout << "errrrrrrrrrrrrrrror" << std::endl;
		}
		pck.clear();
	}
	std::cout << "desconectao" << std::endl;
}

void pingFunction(sf::UdpSocket* sock, std::vector<clientProxy> &clients, sql::Driver* driver, sql::Connection* con)
{
	sf::Packet pckPing;
	int idPacket;
	int begin = 0;
	while (true)
	{
		for (size_t i = 0; i < clients.size(); i++)
		{
			//std::cout << "ping";
			pckPing << PING;
			clients[i].numPings += 1;
			//std::cout << "numeroPings:" << clients[i].numPings;
			
			if (clients[i].numPings >= 4)
			{
				clients[i].numPings = 0;
				disconnectPlayer(sock,i, clients, driver, con);
				
				//sock->close();
			}
			if (i<clients.size()) 
			{
				if (sock->send(pckPing, clients[i].ip, clients[i].port) != sf::Socket::Done)
				{
					//std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
				}
			}
			
			pckPing.clear();
 		}

		Sleep(PINGTIME);
	}


}


void gameFunction(sf::UdpSocket* sock, std::vector<clientProxy> &clients, std::vector<rocasServidor>&obstaculos_)
{
	sf::Packet pckGame;
	int idPacket;

	while (true)
	{
		for (size_t i = 0; i < clients.size(); i++)
		{
			//std::cout << "ping";
			pckGame << UPDATEGAME;
			clients[i].numPings += 1;
			//std::cout << "numeroPings:" << clients[i].numPings;
			
			if (obstaculos_.size() == 2)
			{
				//pckPing << obstaculosServidor.size();
				//std::cout << "SIZE SERVER: " << obstaculosServidor.size();
				for (int i = 0; i < obstaculos_.size(); i++) { //enviamos el vector de rocas actualizado
					pckGame << obstaculos_[i].idRock << obstaculos_[i].color << obstaculos_[i].posX << obstaculos_[i].posY;
					//std::cout << "IDROCAS :" << obstaculos_[i].idRock << std::endl;
					//std::cout <<" color: " << obstaculosServidor[i].color << " posX: " << obstaculosServidor[i].posX << " posY: " << obstaculosServidor[i].posY << std::endl;
				}
				std::cout << pckGame;
				if (sock->send(pckGame, clients[i].ip, clients[i].port) != sf::Socket::Done)
				{
					std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
				}
				pckGame.clear();
			}

		}
		obstaculos_.clear();
		Sleep(UPDATETIME);
	}
}

bool findPlayer(std::vector<clientProxy> clients, clientProxy toCompare)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].alias == toCompare.alias) return true;
	}
	return false;

};
int pckCounter;

void insertPlayer(clientProxy &toCompare, sql::Driver* driver, sql::Connection* con);

void endGame(clientProxy client, sql::Driver* driver, sql::Connection* con)
{

}



void createSesion(clientProxy client, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("INSERT INTO Sesiones(sesionID, idPlayer, Wins, Loses, PUResta, PUsuma) VALUES(NULL,?,?,?,?,?)");

	pstm->setInt(1, client.idPlayer);
	pstm->setInt(2, 0);
	pstm->setInt(3, 0);
	pstm->setInt(4, 0);
	pstm->setInt(5, 0);
	pstm->execute();

}



int getIDSesion(clientProxy client, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("SELECT `sesionID` FROM `Sesiones` WHERE `idPlayer` =?");

	pstm->setInt(1, client.idPlayer);
	pstm->execute();
	res = pstm->getResultSet();

	if (res->next())
	{
		int id;
		do
		{
			id = res->getInt(1);
			//std::cout << "-----" << id << std::endl;

		} while (res->next());
		return id;
	}

}

void closeSesion(clientProxy client, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("UPDATE Sesiones SET Wins = ?, Loses = ?, PUResta = ?, PUsuma = ? WHERE sesionID = ? ");

	pstm->setInt(1, client.victory);
	pstm->setInt(2, client.defeat);
	pstm->setInt(3, client.pwpMinus);
	pstm->setInt(4, client.pwpPlus);
	pstm->setInt(5, getIDSesion(client, driver, con));
	pstm->execute();
}

void getPwerUp(sql::Driver* driver, sql::Connection* con, std::vector<powerUp> &pwp)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("SELECT * FROM `PowerUps`");

	pstm->execute();
	res = pstm->getResultSet();

	powerUp aux;

	if (res->next())
	{
		do
		{
			aux.point = res->getInt(2);
			aux.probabilidad = res->getInt(3);
			pwp.push_back(aux);

		} while (res->next());

	}

}

void cerrarPartida(clientProxy client, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("UPDATE Partidas SET timeOut = NOW() WHERE idSesion=?");
	int idsesion = getIDSesion(client, driver, con);
	pstm->setInt(1, idsesion);
	pstm->execute();

}

void createPartida(clientProxy client, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("INSERT INTO Partidas (partidaID, idSesion) VALUES(NULL,?)");

	int idsesion = getIDSesion(client, driver, con);

	pstm->setInt(1, idsesion);
	pstm->execute();


	std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << std::endl;
}

void getIDPlayer(clientProxy &toCompare, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("SELECT `playerID` FROM `Players` WHERE `userName` =?");
	sql::SQLString userNameBDD = toCompare.alias.c_str();
	pstm->setString(1, userNameBDD);
	pstm->execute();
	res = pstm->getResultSet();

	if (res->next())
	{
		do
		{
			int id = res->getInt(1);
			toCompare.idPlayer = id;
		} while (res->next());
	}
	else
	{
		insertPlayer(std::ref(toCompare), driver, con);		
	}
	delete res;
	delete stm;
	delete pstm;
}

void insertPlayer(clientProxy &toCompare, sql::Driver* driver, sql::Connection* con)
{
	sql::Statement* stm = con->createStatement();
	sql::PreparedStatement* pstm;
	sql::ResultSet* res;

	pstm = con->prepareStatement("INSERT INTO Players(playerID, userName) VALUES(NULL,?)");
	sql::SQLString userNameBDD = toCompare.alias.c_str();
	pstm->setString(1, userNameBDD);
	pstm->execute();

	getIDPlayer(std::ref(toCompare), driver, con);
}

void pckFunction(sf::UdpSocket* sock, std::vector<clientProxy> &clients, std::map<int, sf::Packet> &packetescriticos, std::vector<int> &toErase)
{
	while (true)
	{
		for (int i = 0; i < toErase.size(); i++)
		{

			packetescriticos.erase(toErase[i]);
		}
		toErase.clear();

		mtx.lock();
		for (std::map<int, sf::Packet>::iterator it = packetescriticos.begin(); it != packetescriticos.end(); it++)
		{
			for (int i = 0; i < clients.size(); i++)
			{
				sock->send(it->second, clients[i].ip, clients[i].port);
			}
		}
		mtx.unlock();

		Sleep(PCKTIME);
	}
}

void receiveFunction(sf::UdpSocket* sock, std::vector<clientProxy> &clients, std::map<int, sf::Packet> &packetescriticos, std::vector<int> &toErase, sql::Driver* driver, sql::Connection* con, std::vector<rocasServidor>&_obstaculos, std::vector<players> &_Clientes)
{
	int nPlayers = 0;
	sf::Packet pckToReceive;
	sf::Packet pckToSend;
	AccumMovementServer accum;
	std::vector<AccumMovementServer>Movements;
	int commandToSend, commandToReceive;
	int idMove;
	//int DeltaX;
	//Movement
	int AcumX, AcumY;
	int idPlayer;

	int id_pck;
	//Time
	sf::Clock deltaClock;
	sf::Time prevTime = deltaClock.getElapsedTime();

	//collision
	int idRoca, tipoCol;
	int f = 0;

	while (true)
	{
		clientProxy temp;
		players p;
		pckToReceive.clear();
		sf::UdpSocket::Status status = sock->receive(pckToReceive, temp.ip, temp.port);

		pckToReceive >> commandToReceive;
		std::cout << "cmd: " << commandToReceive << std::endl;

		//Time
		sf::Time elapsedTime = deltaClock.getElapsedTime() - prevTime;

		if (status == sf::UdpSocket::Status::Done)
		{
			//std::cout << "ESTOY RECIVIENDO COMANDOS" << std::endl;
			switch (commandToReceive)
			{
				//NUEVO CLIENTE
			case HELLO:
				//std::cout << "ME HAN ENVIADO HELLO" << std::endl;
				//CREO UN CLIENTPROXY TEMPORAL
				pckToReceive >> temp.alias;
				temp.pos.x = (rand() % 504);
				temp.pos.y = 58;
				temp.numPings = 0;
				temp.points = 0;
				p.alias = temp.alias;
				p.playerid = temp.idPlayer;
				std::cout << "PASA X AQUI" << std::endl;
				if (!findPlayer(clients, temp) && (nPlayers <= 2))
				{
					//std::cout << "Soy nuevo" << std::endl;
					getIDPlayer(std::ref(temp), driver, con);
					createSesion(temp, driver, con);
					createPartida(temp, driver, con);


					temp.pos = { temp.pos.x,  temp.pos.y };
					p.pos = { temp.pos.x, temp.pos.y };

					//Al nuevo conectado le envio su idplayer con su posicion
					commandToSend = WELCOME;
					pckToSend << commandToSend << temp.alias << temp.pos.x << temp.pos.y << temp.idPlayer << temp.points;
					if (sock->send(pckToSend, temp.ip, temp.port) != sf::Socket::Done)
					{
						std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
					}
					
				}
				//NOTWELCOME
				else
				{
					//std::cout << "No soy Nuevo" << std::endl;
					std::cout << "NOTWELCOME" << std::endl;
					commandToSend = NOTWELCOME;
					pckToSend << commandToSend;
					if (sock->send(pckToSend, temp.ip, temp.port) != sf::Socket::Done)
					{
						std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
					}
					temp.alias = "";
					
				}
				pckToReceive.clear();
				pckToSend.clear();
				commandToReceive = -1;
				break;
			case ACKNWELLCOME:
			{
				pckToReceive >> temp.alias;
				pckToReceive >> temp.pos.x;
				pckToReceive >> temp.pos.y;
				pckToReceive >> temp.idPlayer;
				pckToReceive >> temp.points;

				if (!findPlayer(clients, temp) && (nPlayers <= 2))
				{
					clients.push_back(temp);
					nPlayers++;

					//A los diferentes jugadores les envio el nuevo que ha entrado 
					for (int i = 0; i < (clients.size() - 1); i++)
					{
						commandToSend = NEWPLAYER;
						pckToSend << commandToSend << temp.idPlayer << temp.pos.x << temp.pos.y;

						if (sock->send(pckToSend, clients[i].ip, clients[i].port) != sf::Socket::Done)
						{
							std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
						}
						pckToSend.clear();
					}

					//Al nuevo jugador le envio los jugadores existentes
					for (int i = 0; i < (clients.size() - 1); i++)
					{
						commandToSend = NEWPLAYER;
						pckToSend << commandToSend << clients[i].idPlayer << clients[i].pos.x << clients[i].pos.y;

						if (sock->send(pckToSend, temp.ip, temp.port) != sf::Socket::Done)
						{
							std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
						}
						pckToSend.clear();
					}

					pckToReceive.clear();
					id_pck = -1;
					commandToReceive = -1;
					break;
				}
				else
				{
					std::cout << "NOTWELCOME" << std::endl;
					commandToSend = NOTWELCOME;
					pckToSend << commandToSend << pckCounter;
					if (sock->send(pckToSend, temp.ip, temp.port) != sf::Socket::Done)
					{
						std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
					}
					
				}
				pckToReceive.clear();
				pckToSend.clear();
				commandToReceive = -1;
				break;
			}
				
			case ACKN:
			{
				pckToReceive >> id_pck;
				toErase.push_back(id_pck);
				pckToReceive.clear();
				id_pck = -1;
				commandToReceive = -1;
				break;
			}
			case MOVEMENT: 
			{

				std::cout << "Estoy recibiendo movimiento" << std::endl;
				int size = 0;
				pckToReceive >> size;
				for (int i = 0; i<clients.size(); i++)
				{
					if (clients[i].port == temp.port)//buscamos que cliente nos esta enviando este acumulado
					{
						accum.idPlayer = clients[i].idPlayer;

						for (int j = 0; j < size; j++)
						{
							pckToReceive >> accum.idMove;
							pckToReceive >> accum.AbsoluteX;
							pckToReceive >> accum.AbsoluteY;

							accums.push_back(accum); //acumulamos los movimientos en un vector de acumulados
						}
						mtx.lock();
						mapMovements[clients[i].idPlayer] = accums;//rellenamos el map con el vector anterior
						mtx.unlock();
					}
				}
				pckToReceive.clear();
				commandToReceive = -1;
				break;
			}
			case PINGACKN:
			{
				std::cout << "ping ack recibido:" << std::endl;
				int PLAYERID = -1;
				pckToReceive >> PLAYERID;
				//std::cout << "PlayerId:" <<PLAYERID<< std::endl;
				//std::cout << "numnberOfPings:" << clients[PLAYERID].numPings << std::endl;
				if (PLAYERID != -1) {
					//std::cout << "Aqui estoy" << std::endl;
					for (int i = 0; i < clients.size(); i++) {
						if (clients[i].idPlayer == PLAYERID) {
							clients[i].numPings = 0;
							std::cout << "numeroPingsRESET:" << clients[i].numPings;
						}
					}
				}
				//std::cout << "numnberOfPings2:" << clients[PLAYERID].numPings << std::endl;
				pckToReceive.clear();
				break;
			}

			default:
				break;
			}
		}
	}
}
void updateRocks(sf::RenderWindow &window, std::vector<rocasServidor> &obstaculos, std::vector<powerUp> &pwp) {
	rocasCliente rc;
	rocasServidor r;
	static int idRoca = 0;
	//int id = -1;
	r.idRock = 0;
	if (obstaculos.size() < 2) {
		for (int i = 0; i < 2; i++)
		{
			idRoca += 1;
			r.idRock = idRoca;
			rc.idRock = idRoca;
			//std::cout << "DIBUJANDO ROCAS" << std::endl;
			int color = rand() % 64;
			if (color> pwp[0].probabilidad && color <  pwp[1].probabilidad) {
				r.color = 0; //RED
				rc.roca.setFillColor(sf::Color(255, 0, 0));
				rc.roca.setSize(sf::Vector2f(8, 8));;
			}
			if (color <  pwp[0].probabilidad) {
				r.color = 1; //GREEN
				rc.roca.setFillColor(sf::Color(0, 255, 0));
				rc.roca.setSize(sf::Vector2f(8, 8));;
			}
			if (color> pwp[1].probabilidad && deadRocks == true ) {
				r.color = 2; //GOLD
				rc.roca.setFillColor(sf::Color(255, 215, 0));
				rc.roca.setSize(sf::Vector2f(8, 8));;
			}
			r.posX = (rand() % 512);
			r.posY = -(70);
			rc.posX = r.posX;
			rc.posY = r.posY;

			rc.roca.setPosition(sf::Vector2f(r.posX*8., r.posY*8.));

			obstaculos.push_back(r); //para enviar 
			obstaculosServidor.push_back(rc);	//para actualzar en servidor
		}


		//for (int i = 0; i < obstaculosServidor.size(); i++) { //actualizamos en servidor

		//	obstaculosServidor[i].posY += 1;
		//	obstaculosServidor[i].roca.setPosition(8 * (float)obstaculosServidor[i].posX, 8 * (float)obstaculosServidor[i].posY);
		//	if (obstaculosServidor[i].posY>= 64) {
		//		obstaculosServidor.erase(obstaculosServidor.begin() + i);
		//	}
		//}


	}
}

sf::Vector2f BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}

void DibujaSFML()
{

	sf::Vector2f casillaOrigen, casillaDestino;
	bool casillaMarcada = false;

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

		//Para pintar el un circulito
		sf::CircleShape shape(RADIO_AVATAR);
		shape.setFillColor(sf::Color::Blue);
		sf::Vector2f posicion_bolita(4.f, 7.f);
		posicion_bolita = BoardToWindows(posicion_bolita);
		shape.setPosition(posicion_bolita);
		window.draw(shape);
		window.display();
	}

}

void checkColision(sf::UdpSocket* sock, std::vector<clientProxy> &_Clientes, std::vector<rocasCliente> &obstaculos, std::vector<powerUp> &pwp, sql::Driver* driver, sql::Connection* con) {
	sf::RectangleShape player(sf::Vector2f(8.f, 8.f));
	sf::Packet pckToSend;
	int type;
	int cmd;
	for (int i = 0; i < obstaculos.size(); i++)
	{

		if (obstaculos[i].posY >= 58)
		{
			for (int j = 0; j < _Clientes.size(); j++)
			{
				player.setPosition(sf::Vector2f(_Clientes[j].pos.x , _Clientes[j].pos.y*8.));

				if (obstaculos[i].roca.getGlobalBounds().intersects(player.getGlobalBounds()))
				{
					if (obstaculos[i].roca.getFillColor() == sf::Color(255, 215, 0)) {

						cmd = ENDGAME; // gold fin de partida
						finDeljuego = true;
						_Clientes[j].defeat++;
						cerrarPartida(_Clientes[j], driver, con);
						if (j == 1)
						{
							_Clientes[j - 1].victory++;
							cerrarPartida(_Clientes[j - 1], driver, con);
						}
						else
						{
							_Clientes[j + 1].victory++;
							cerrarPartida(_Clientes[j + 1], driver, con);
						}


					}
					if (obstaculos[i].roca.getFillColor() == sf::Color(255, 0, 0)) {
						cmd = COLLISION;
						_Clientes[j].points -= pwp[1].point;
						_Clientes[j].pwpMinus++;
						type = 1; // gold fin de partida
					}
					if (obstaculos[i].roca.getFillColor() == sf::Color(0, 255, 0)) {
						cmd = COLLISION;
						_Clientes[j].points += pwp[0].point;
						_Clientes[j].pwpPlus++;
						type = 2; // gold fin de partida
					}
					int idPlayer = _Clientes[j].idPlayer;
					pckToSend << cmd <<idPlayer<< obstaculos[i].idRock << _Clientes[0].idPlayer << _Clientes[0].points << _Clientes[1].idPlayer << _Clientes[1].points;
					for (int i = 0; i < _Clientes.size(); i++) 
					{
						if (sock->send(pckToSend, _Clientes[i].ip, _Clientes[i].port) != sf::Socket::Done)
						{
							std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
						}
						else {
							std::cout << "COLLISIONNN" << std::endl;
						}
					}
					obstaculos.erase(obstaculos.begin() + i); //eliminamos el obstaculo en servidor
				}

			}

			pckToSend.clear();
		}

	}



}

void drawRocks(sf::RenderWindow &window, std::vector<rocasCliente> &obstaculos) {
	for (int i = 0; i < obstaculos.size(); i++) {

		obstaculos[i].posY += 1;
		obstaculos[i].roca.setPosition((float)obstaculos[i].posX,  (float)obstaculos[i].posY);
		window.draw(obstaculos[i].roca);

		/*if (obstaculos[i].posY >= 64) {

		///std::cout << "Eliminando rocas" << std::endl;
		obstaculos.erase(obstaculos.begin() + i);
		}*/
	}
}
void drawPlayers(sf::RenderWindow &window, std::vector<clientProxy> &_Clientes)
{
	//std::cout << _Clientes.size() << std::endl;

	sf::CircleShape shape;
	shape.setFillColor(sf::Color(255, 0, 0));
	shape.setRadius(4);
	for (int i = 0; i < _Clientes.size(); i++)
	{
		if (i == 0) shape.setFillColor(sf::Color(128, 0, 128));
		else shape.setFillColor(sf::Color(0, 0, 255));
		shape.setPosition(sf::Vector2f( _Clientes[i].pos.x, 8 * _Clientes[i].pos.y));
		window.draw(shape);
	}

}
struct points {
	int posy, posx;
};

void drawGrid(sf::RenderWindow &window)
{
	sf::RectangleShape rectangle;
	rectangle.setFillColor(sf::Color(255, 0, 0));
	rectangle.setSize(sf::Vector2f(8, 8));
	bool color = true;

	window.clear();
	color = true;
	for (int i = 0; i < 63; i++)
	{
		for (int j = 0; j < 63; j++)
		{

			rectangle.setFillColor(sf::Color(0.5f, 0.5f, 0.5f));
			color = false;
			rectangle.setPosition(sf::Vector2f(8 * j, 8 * i));
			window.draw(rectangle);
		}
	}
}

void  main()
{
	finDeljuego = false;
	deadRocks = false;
	sql::Driver* driver = get_driver_instance();
	sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
	con->setSchema(DATABASE);
	std::cout << "Base  de datos abierta, pueden iniciar clientes" << std::endl;

	sf::UdpSocket sock;
	sf::UdpSocket::Status st = sock.bind(50000);

	std::vector<powerUp> pwups;
	getPwerUp(driver, con, std::ref(pwups));


	std::vector<players> Clientes;
	std::map<int, sf::Packet> packetescriticos;
	std::vector<int> pckToErase;

	std::vector <clientProxy> Proxys;

	std::thread pckCriticosThread(&pckFunction, &sock, std::ref(Proxys), std::ref(packetescriticos), std::ref(pckToErase));
	pckCriticosThread.detach();

	std::thread movementThread(&validateMovement, &sock, std::ref(Proxys)); //thread para validar el movimiento
	movementThread.detach();

	std::thread pingThread(&pingFunction, &sock, std::ref(Proxys), driver, con);
	pingThread.detach();

	std::thread gameThread(&gameFunction, &sock, std::ref(Proxys), std::ref(obstaculosServidorAux));
	gameThread.detach();

	std::thread receiveThread(&receiveFunction, &sock, std::ref(Proxys), std::ref(packetescriticos), std::ref(pckToErase), driver, con, std::ref(obstaculosServidorAux), std::ref(Clientes));
	receiveThread.detach();

	//std::map<int, clientProxy> mapClients;
	sf::Vector2f casillaOrigen, casillaDestino;
	bool casillaMarcada = false;
	sf::RenderWindow window(sf::VideoMode(512, 512), "Ejemplo tablero");
	//time
	int time = 0;

	if (st == sf::UdpSocket::Status::Done)
	{
		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event)) {
				//std::cout << Clientes.size() << std::endl;
				switch (event.type)
				{
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::S) { //si activas la s desde servidor aparecen los cubos amarillos que te matan 
						deadRocks = true;
					}
					break;
				default:
					break;
				}
			}
			while (window.pollEvent(event))
			{
				// "close requested" event: we close the window
				if (event.type == sf::Event::Closed)
					window.close();
			}

			drawGrid(window);

			if (time >= 200 && Clientes.size()<=2) {
				updateRocks(window, std::ref(obstaculosServidorAux), std::ref(pwups));
			}
			if (Proxys.size() >= 2 && finDeljuego==false)
			{
				time++;
				if (time <= 200)
				{
					time++;				
				}
				drawPlayers(window, std::ref(Proxys));
				drawRocks(window, std::ref(obstaculosServidor));
				checkColision(&sock, std::ref(Proxys), std::ref(obstaculosServidor), std::ref(pwups), driver, con);
				window.display();
			}
			
		}
	}
}
