#include <iostream>
#include <thread>
#include <SFML\Graphics.hpp>
#include <PlayerInfo.h>
#include <Windows.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
std::vector<players> Clientes;
int aux_x;
int dest;
void interpolateRival(int destiny);

sf::Mutex mtx;

void acknowledgePing(sf::UdpSocket* sock, players &local)
{
	
	sf::Packet pck_ack;
	sf::IpAddress ip = sf::IpAddress::getLocalAddress(); //change it!
	std::string str;
	pck_ack << PINGACKN<< local.playerid;

	//std::cout << "PlayerId" <<local.playerid<< std::endl;
	if (sock->send(pck_ack, ip, 50000) != sf::Socket::Done)
	{
		std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
	}
	else {
		//std::cout << "Se ha enviado PINGACKN" << std::endl;
	}
	pck_ack.clear();
}

void acknowledge(sf::UdpSocket* sock, players _local, msgType command)
{
	sf::Packet pck_ack;
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	std::string str;

	switch (command)
	{
	default:
		break;
	case ACKNWELLCOME:
		pck_ack << ACKNWELLCOME << _local.alias << _local.pos.x << _local.pos.y << _local.playerid << _local.points;
		break;
	}

	if (sock->send(pck_ack, ip, 50000) != sf::Socket::Done)
	{
		std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
	}
	pck_ack.clear();
}

void receiveThread(sf::UdpSocket* sock, players &local, bool &endgame, sf::Text &text, std::vector<AccumMovement>&accumMovements, bool &_isWelcome)
{
	sf::Packet pckToReceive;
	sf::Packet pckToSend;

	int commandToReceive;
	int commandToSend;

	int pck_id;
	std::string alias;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	std::string str;

	//Movemet
	int  newPosX;
	int newPosY;
	int idPlayer;

	//Map
	int id;
	int color;
	rocasCliente r;
	int size;
	//collision
	int idRock;
	int idRockRecived;
	int t = 0;
	int idPlayerOne;
	int PointsPlayerOne;
	int idPlayerTwo;
	int PointsPlayerTwo;
	int idP;
	int end_game = false;
	//std::vector<rocasCliente>aux;
	while (true)
	{		
		sf::UdpSocket::Status status = sock->receive(pckToReceive, senderIP, senderPort);
		players newPlayer;

		if (status == sf::UdpSocket::Status::Done)
		{					
			pckToReceive >> commandToReceive;
			//std::cout << "cmd" << commandToReceive << std::endl;
			
			switch (commandToReceive)
			{
			case WELCOME:	
				_isWelcome = true;
				pckToReceive >> local.alias;
				pckToReceive >> local.pos.x;
				pckToReceive >> local.pos.y;
				pckToReceive >> local.playerid;
				pckToReceive >> local.points;
				Clientes.push_back(local);
				pckToReceive.clear();
				std::cout << "WELCOME: " << local.alias<<" playerId es: "<< local.playerid<< std::endl;
				acknowledge(sock, local, ACKNWELLCOME);
				commandToReceive = -1;
				pck_id = -1;
				pckToReceive.clear();
				break;
			case NOTWELCOME:
				_isWelcome = true;
				pckToReceive >> newPlayer.alias;
				pckToReceive >> newPlayer.pos.x;
				pckToReceive >> newPlayer.pos.y;
				pckToReceive.clear();
				std::cout << "YOU ALREADY IN "<< std::endl;
				commandToReceive = -1;
				pck_id = -1;
				pckToReceive.clear();
				exit(0);
				break;
			case NEWPLAYER:
				pckToReceive >> newPlayer.playerid;				
				pckToReceive >> newPlayer.pos.x;
				pckToReceive >> newPlayer.pos.y;
				pckToReceive.clear();
				Clientes.push_back(newPlayer);
				std::cout << "WELCOME: " << newPlayer.alias << " playerId es: " << newPlayer.playerid << std::endl;
				acknowledge(sock, newPlayer, NEWPLAYER);
				commandToReceive = -1;
				pck_id = -1;
				pckToReceive.clear();
				break; 
			case OKMOVEMENT:
				std::cout << "El servidor ha verificado el movimiento" << std::endl;
				int idMove;
				pckToReceive >> idMove;
				aux_x = local.pos.x;
				mtx.lock();
				for (int i = 0; i<accumMovements.size(); i++)
				{
					if (accumMovements[i].idMove == idMove)
					{
						accumMovements.erase(accumMovements.begin(), accumMovements.begin() + i + 1); //Eliminamos el movimiento que ya ha sido realizado
					}
				}
				mtx.unlock();
				pckToReceive.clear();
				commandToReceive = -1;
				break;
			case BADMOVEMENT:
				int id;
				//std::cout << "Estoy recibiendo MAL MOVE" << std::endl;
				pckToReceive >> id;
				std::cout << "id:" << id << std::endl;
				for (int i = 0; i< accumMovements.size(); i++)
				{
					if (accumMovements[i].idMove == id)
					{
						std::cout << "ENTRO" << std::endl;
						if (i == 0)
						{
							if (Clientes[0].pos.x>50)
							{
								Clientes[0].pos.x = 60;
							}
							if (Clientes[0].pos.x<20)
							{
								Clientes[0].pos.x = 5;
							}
						}
						else {

							// std::cout << "posXInit:" << Clientes[0].pos.x << std::endl;
							Clientes[0].pos.x = accumMovements[i - 1].AbsoluteX;
							//std::cout << "posXLast:" << Clientes[0].pos.x << std::endl;


							accumMovements.erase(accumMovements.begin(), accumMovements.begin() + i + 1); //Eliminamos el movimiento que ya ha sido realizado
						}
					}
				}

				pckToReceive.clear();
				commandToReceive = -1;
				break;
			case MOVEMENT:
				dest = 0;
				pckToReceive >> dest;

				//Clientes[1].pos.x = x;
				pckToReceive.clear();
				commandToReceive = -1;
				break;
			case PING: //Gestion del ping 
				//std::cout << "PING RECIBIDO" << std::endl;
				acknowledgePing(sock, std::ref(local));
				pckToReceive.clear();
				commandToReceive = -1;
				//exit(0);
				break;
			case UPDATEGAME: // update del juego
				interpolateRival(dest); //interpolamos la posición del rival

				for (int i = 0; i < 2; i++) {
					pckToReceive >> r.idRock;
					pckToReceive >> color;
					pckToReceive >> r.posX;
					pckToReceive >> r.posY;

					if (color == 0) {
						r.roca.setFillColor(sf::Color(255, 0, 0));
						r.roca.setSize(sf::Vector2f(8, 8));;

					}
					else if (color == 1) {
						r.roca.setFillColor(sf::Color(0, 255, 0));
						r.roca.setSize(sf::Vector2f(8, 8));;
					}
					else {
						r.roca.setFillColor(sf::Color(255, 215, 0));
						r.roca.setSize(sf::Vector2f(8, 8));;
					}

					//std::cout << "color: " << color << std::endl;
					//std::cout << "posX: " << r.posX << std::endl;
					//std::cout << "posY: " << r.posY << std::endl;


					r.roca.setPosition(sf::Vector2f(r.posX*8., r.posY*8.));

					obstaculosCliente.push_back(r);
				}

				//obstaculosCliente = aux; //se actualiza el vector de rocas en cliente

				//std::cout << "PNG ON" << std::endl;

				pckToReceive.clear();
				commandToReceive = -1;
				//exit(0);
				break;
			case COLLISION:
				std::cout << "ME ENVIAN COLLISION" << std::endl;
				pckToReceive >> idPlayer;
				pckToReceive >> idRock;
				pckToReceive >> idPlayerOne;
				pckToReceive >> PointsPlayerOne;
				pckToReceive >> idPlayerTwo;
				pckToReceive >> PointsPlayerTwo;

				for (int i = 0; i < Clientes.size(); i++) { // Actualizamos las puntuaciones de los clientes
					if (Clientes[i].playerid == idPlayerOne) {
						Clientes[i].points = PointsPlayerOne;
					}
					else if(Clientes[i].playerid == idPlayerTwo){
						Clientes[i].points = PointsPlayerTwo;
					}
				}
				/*for (int j = 0; j < obstaculosCliente.size(); j++) { // Actualizamos las puntuaciones de los clientes
					if (obstaculosCliente[j].idRock == idRock) {
						obstaculosCliente.erase(obstaculosCliente.begin() + j);
					}
				}*/
				
				//obstaculosCliente.erase(obstaculosCliente.begin() + idRock);
				pckToReceive.clear();
				commandToReceive = -1;
				break;
			case ENDGAME:
				//std::cout << "ENDGAME" << std::endl;
				pckToReceive >> idPlayer;
				pckToReceive >> idRock;
				pckToReceive >> idPlayerOne;
				pckToReceive >> PointsPlayerOne;
				pckToReceive >> idPlayerTwo;
				pckToReceive >> PointsPlayerTwo;

				if (end_game == false) {
					//std::cout << "idPlayer" << idPlayer << std::endl;
					//std::cout << "localPlayerID" << local.playerid << std::endl;
					for (int i = 0; i < Clientes.size(); i++) { // Actualizamos las puntuaciones de los clientes
						if (Clientes[i].playerid == idPlayerOne) {
							Clientes[i].points = PointsPlayerOne;
						}
						else if (Clientes[i].playerid == idPlayerTwo) {
							Clientes[i].points = PointsPlayerTwo;
						}
					}
					for (int j = 0; j < obstaculosCliente.size(); j++) { // Actualizamos las puntuaciones de los clientes
						if (obstaculosCliente[j].idRock == idRock) {
							obstaculosCliente.erase(obstaculosCliente.begin() + j);
						}
					}

					if (idPlayer == local.playerid) {
						local.victory = 0;
						text.setString("YOU LOSE");
						std::string r = text.getString();
						std::cout << r << std::endl;

					}
					else if (idPlayer != local.playerid) {
						local.victory = 1;
						text.setString("YOU WIN");
						std::string r = text.getString();
						std::cout << r << std::endl;
					}
				}

				endgame = true;
				
				//obstaculosCliente.erase(obstaculosCliente.begin() + idRock);
				pckToReceive.clear();
				commandToReceive = -1;
				break;
			case ENDGAMEDISCONECT:
				std::cout << "TU GANAS" << std::endl;
				endgame = true;
				local.victory = 1;
				text.setString("YOU WIN");
				//std::string ra = text.getString();

				pckToReceive.clear();
				commandToReceive = -1;
				break;
			default:
				break;
			}
		}
	}
}

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

void drawRocks(sf::RenderWindow &window, std::vector<rocasCliente> &obstaculos) {
		
	for (int i = 0; i < obstaculos.size(); i++) {

		obstaculos[i].posY += 1;
		obstaculos[i].roca.setPosition( obstaculos[i].posX, obstaculos[i].posY);
		window.draw(obstaculos[i].roca);
		/*if (obstaculos[i].posY >= 64) {

			///std::cout << "Eliminando rocas" << std::endl;
			obstaculos.erase(obstaculos.begin() + i);
		}*/
	}
}
void drawPlayers(sf::RenderWindow &window, std::vector<players> &_Clientes )
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
	window.display();
}
struct points {
	int posy, posx;
};




void main() 
{
	bool isWelcome = false;
	//Movement
	AccumMovement ac;
	ac.idMove = 0; ac.AbsoluteX = 0; ac.AbsoluteY = 0;
	srand(time(NULL));
	std::vector<AccumMovement>Movements;
	//EndGame
	bool end_game = false;
	//Init
	std::string str;
	std::cout << "Introduce un alias: ";
	std::cin >> str;
	
	int velocity;

	sf::UdpSocket sock;
	sf::Packet pck;
	pck<< HELLO << str;
	
	sf::IpAddress a = sf::IpAddress::getLocalAddress();
	
	//Creo vector de clientes e inicializo el propio cliente
	

	players local;
	sf::Socket::Status status = sock.send(pck, a, 50000);
	if (status != sf::Socket::Done)
	{
		std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
	}
	
	pck.clear();


	sf::Font font;

	if (!font.loadFromFile("cour.ttf"))
	{
		std::cout << "eeeeerrrrrrrror Font" << std::endl;
	}

	sf::Text textEnd;
	textEnd.setFont(font);
	textEnd.setCharacterSize(24);
	textEnd.setFillColor(sf::Color::Yellow);
	textEnd.setStyle(sf::Text::Bold | sf::Text::Underlined);
	textEnd.setPosition(sf::Vector2f(25.f *8.f, 32.0f*8.f));



	sf::Text textPoints;
	textPoints.setFont(font);
	textPoints.setCharacterSize(24);
	textPoints.setFillColor(sf::Color::Red);
	textPoints.setStyle(sf::Text::Bold | sf::Text::Underlined);
	textPoints.setPosition(sf::Vector2f(40.f*8.f, 0.f));

	std::thread thread(&receiveThread, &sock, std::ref(local), std::ref(end_game), std::ref(textEnd), std::ref(Movements), std::ref(isWelcome));
	thread.detach();

	while (isWelcome == false)
	{
		std::cout << isWelcome << std::endl;
		std::cout << "Send Hello" << std::endl;
		status = sock.send(pck, a, 50000);
		Sleep(1000);
	}
	pck.clear();
	if (status != sf::Socket::Done)
	{
		std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
	}
	else {
		std::cout << "aciertooooo";

	}
	pck.clear();

	sf::RenderWindow window(sf::VideoMode(504, 504), "Ejemplo tablero");	
	//std::vector<points>Points;
	sf::Vector2i pos;
	sf::Clock deltaClock;
	sf::Time prevTime = deltaClock.getElapsedTime();	

	//Puntos Jugador
	int Points = 0;

	//Text alias
	sf::Text text;
	text.setFont(font);
	text.setString(local.alias);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::Red);
	text.setStyle(sf::Text::Bold | sf::Text::Underlined);

	//text Waiting
	sf::Text textWaiting;
	textWaiting.setFont(font);
	textWaiting.setString("Waiting for players");
	textWaiting.setCharacterSize(18);
	textWaiting.setFillColor(sf::Color::Yellow);
	textWaiting.setStyle(sf::Text::Bold);
	textWaiting.setPosition(sf::Vector2f(20.f *8.f, 32.0f*8.f));
	
	
	pos = local.pos;
	int time = 0;
	int id = 0;
	while(window.isOpen())
	{
		pck.clear();
		sf::Time elapsedTime = deltaClock.getElapsedTime() - prevTime;
		//std::cout << "bool :" << end_game<<std::endl;
		//std::cout << elapsedTime.asMilliseconds() << std::endl;
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::A) {
					if (Clientes.size() == 2) {
						ac.idMove = id;
						id++;
						//std::cout << "LEFT" << std::endl;
						pos.x = pos.x - 8;
						Clientes[0].pos = pos;
						ac.AbsoluteX = pos.x;
						ac.AbsoluteY = pos.y;
						mtx.lock();
						Movements.push_back(ac);
						mtx.unlock();
					}
				}
				if (event.key.code == sf::Keyboard::D) {
					if (Clientes.size() == 2) {
						ac.idMove = id;
						id++;
						//std::cout << "RIGHT" << std::endl;
						pos.x = pos.x + 8;
						Clientes[0].pos = pos;
						ac.AbsoluteX = pos.x;
						ac.AbsoluteY = pos.y;
						mtx.lock();
						Movements.push_back(ac);
						mtx.unlock();
					}
				}if (event.key.code == sf::Keyboard::Space) {
					end_game = false;
				}
			default:
				break;
			}
		}

		drawPlayers(window, std::ref(Clientes));
		drawGrid(window);
		window.draw(text);
		if (Clientes.size()<2) {
			
			window.draw(textWaiting);

		}

		textPoints.setString("POINTS: " + std::to_string(Clientes[0].points));

		window.draw(textPoints);

		//window.clear();
		if (end_game == false) {
			time += 1;			
			drawRocks(window, std::ref(obstaculosCliente));
		}


		if (end_game == true) { //controlamos lo que sale por pantalla
			if (local.victory ==0) {
				textEnd.setString("YOU LOSE");
			}
			else {

				textEnd.setString("YOU WIN");
			}
			window.draw(textEnd);
		}
		
		if (elapsedTime >= sf::milliseconds(1000) && Movements.size() != 0)
		{

			mtx.lock();
			pck << MOVEMENT;
			int aux;
			aux = Movements.size();
			pck << aux;
			for (int i = 0; i < Movements.size(); i++)
			{
				//Rellenamos el pck con los datos del vector de movimientos accumulados. 
				pck << Movements[i].idMove << Movements[i].AbsoluteX << Movements[i].AbsoluteY;

			}
			mtx.unlock();
			aux_x = local.pos.x;
			if (sock.send(pck, a, 50000) != sf::Socket::Done) //enviamos el paquete de movimiento
			{
				std::cout << "errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrror";
			}
			pck.clear();
			prevTime = deltaClock.getElapsedTime(); // reseteamos el tiempo 
		}
	}
	std::cout << "Me he marchado" << std::endl;
	system("pause");
}

void interpolateRival(int destiny)
{
	std::cout << "destiny: " << destiny << std::endl;
	if (Clientes[1].pos.x < destiny && destiny != 0)
	{
		Clientes[1].pos.x += 8;
		std::cout << "DERECHA " << Clientes[1].pos.x << std::endl;
	}
	else if (Clientes[1].pos.x > destiny && destiny != 0)
	{
		std::cout << "IZQUIERDA " << Clientes[1].pos.x << std::endl;
		Clientes[1].pos.x -= 8;
	}

}