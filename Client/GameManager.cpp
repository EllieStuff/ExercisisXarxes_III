#include "GameManager.h"
#include "SceneManager.h"
#include <thread>

void GameManager::CalculateTurn()
{
}

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
}

void GameManager::ConnectP2P()
{

}

void GameManager::SendMessages()
{
}

void GameManager::ReceiveMessages(int _sockIdx)
{
}

void GameManager::AcceptConnections(int* sceneState)
{
	TcpListener listener;
	listener.Listen(localPort);

	while (socks.size() < 3 && *sceneState != (int)SceneManager::Scene::GAMEOVER)
	{
		TcpSocket* sock = new TcpSocket();
		Status status = listener.Accept(*sock);
		if (status == Status::DONE) {
			socks.push_back(sock);
			std::cout << "Connected with ip: " << sock->GetRemoteAddress() << " and port: " << sock->GetLocalPort() << std::endl;

			/*TurnSystem(_socks);*/
			std::thread tReceive(&ReceiveMessages, (int)(socks.size() - 1));
			tReceive.detach();
		}
	}

	listener.Close();
}
