#include "SceneManager.h"
#include "../res/OutputMemoryStream.h"
#include "../res/InputMemoryStream.h"

void SceneManager::EnterInit()
{
	scene = Scene::INIT;

	//Connect to server
	Status status = serverSock.Connect("127.0.0.1", 50000);
	if (status != Status::DONE) {
		return;
	}
	game.SetPort(serverSock.GetLocalPort());


}

void SceneManager::EnterGame()
{
	scene = Scene::GAME;
}

void SceneManager::ExitInit()
{
	serverSock.Disconnect();
}

void SceneManager::ExitGame()
{
	scene = Scene::GAMEOVER;

}

void SceneManager::UpdateInit()
{
	std::cout << "Welcome!" << std::endl;
	std::cout << "1. Create P2P Game" << std::endl;
	std::cout << "2. List P2P Games" << std::endl;
	std::cout << "3. Join P2P Game" << std::endl;

	std::cout << "\nSelect option: ";
	int option;
	std::cin >> option;
	if (option <= 0 || option > 3)
		return;

	OutputMemoryStream* out = new OutputMemoryStream();

	Status status;
	serverSock.Send(out, status);

	if (option == 1)
	{
		out->Write(Commands::CREATE_GAME);
		CreateGame();
	}
	else if (option == 2)
	{
		out->Write(Commands::GAME_LIST);
		ListCurrentGames();
	}
	else if (option == 3)
	{
		out->Write(Commands::JOIN_GAME);
		JoinGame();
	}

	delete out;
}

void SceneManager::UpdateGame()
{
}

void SceneManager::UpdateGameOver()
{
}

void SceneManager::CreateGame()
{

}

void SceneManager::ListCurrentGames()
{
}

void SceneManager::JoinGame()
{
	//Choose game
	std::cout << "Type server ID" << std::endl;
	int server;
	std::cin >> server;

	OutputMemoryStream* out = new OutputMemoryStream();

	Status status;
	serverSock.Send(out, status);

	delete out;

	//Write password (if necessary)
	InputMemoryStream* in;
	in = serverSock.Receive(status);

	std::string msg = in->ReadString();
	delete in;

	//Write message in console
	std::cout << msg << std::endl;

	if (msg != "")
	{
		do
		{
			in = serverSock.Receive(status);

			msg = in->ReadString();
			std::cout << msg << std::endl;

			std::cin >> msg;

			out = new OutputMemoryStream();
			out->WriteString(msg);
			serverSock.Send(out, status);
			delete out;

		} while (msg == "Incorrect password. Try again or write 'exit' to leave");
	}
}

SceneManager::SceneManager()
{
	EnterInit();
}

SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	while (scene != Scene::GAMEOVER)
	{
		switch (scene)
		{
		case SceneManager::Scene::INIT:
			UpdateInit();
			break;
		case SceneManager::Scene::GAME:
			UpdateGame();
			break;
		}
	}
}
