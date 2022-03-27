#include "SceneManager.h"
#include "../res/OutputMemoryStream.h"
#include "../res/InputMemoryStream.h"

void SceneManager::Start()
{
	//Connect to server
	Status status = serverSock.Connect("127.0.0.1", 50000);
	if (status != Status::DONE) {
		return;
	}

	game.SetPort(serverSock.GetLocalPort());

	std::thread tClient(&GameManager::ClientControl, &game, &serverSock);
	tClient.detach();

	sceneState = Scene::START;
}

void SceneManager::EnterGame()
{
	game.Start();

	std::cout << "Waiting For your turn" << std::endl;

	sceneState = Scene::GAME;
}


void SceneManager::ExitGame()
{
	sceneState = Scene::GAMEOVER;

}

void SceneManager::UpdateInit()
{
	std::cout << "\nWelcome!" << std::endl;
	std::cout << "1. Create P2P Game" << std::endl;
	std::cout << "2. List P2P Games" << std::endl;
	std::cout << "3. Join P2P Game" << std::endl;

	std::cout << "\nSelect option: " << std::endl;

	Commands option;
	std::string tmpOption;
	std::cin >> tmpOption;
	//tmpOption -= '0';

	if (tmpOption == "1") option = Commands::CREATE_GAME;
	else if (tmpOption == "2") option = Commands::GAME_LIST;
	else if (tmpOption == "3") option = Commands::JOIN_GAME;
	else return;
	//option = (Commands)tmpOption;

	if (option < Commands::CREATE_GAME || option > Commands::JOIN_GAME)
		return;

	OutputMemoryStream* out = new OutputMemoryStream();
	Status status;
	out->Write(option);

	bool aborted = false;
	if (option == Commands::CREATE_GAME)
	{
		std::cout << "Create game asked" << std::endl;
		game.CreateGame(out);
		serverSock.Send(out, status);
	}
	else if (option == Commands::GAME_LIST)
	{
		std::cout << "Game list asked" << std::endl;
		serverSock.Send(out, status);
	}
	else if (option == Commands::JOIN_GAME)
	{
		std::cout << "Join game asked" << std::endl;
		game.JoinGame(out, aborted);
		serverSock.Send(out, status);
	}
	
	if (option == Commands::CREATE_GAME || (option == Commands::JOIN_GAME && !aborted)) {
		
		std::cout << "Waiting for players" << std::endl;

		while (game.GetGameSize() < 0 ||( game.GetPlayersNum() < game.GetGameSize())) {}

		std::cout << game.GetPlayersNum() << ", " << game.GetGameSize() << std::endl;

		while (game.GetPlayersReady() < game.GetPlayersNum())
		{
			if (game.GetReady()) continue;
			std::cout << "Are you ready? (Y/N) " << game.GetPlayersReady() << std::endl;
			std::string _ready;
			std::cin >> _ready;

			if (!(_ready == "Y" || _ready == "y")) continue;

			game.SetReady();

			std::cout << "I'm ready!!!!" << std::endl;
		}

		EnterGame();
	}
}

void SceneManager::UpdateGame()
{
	if (game.Update())
	{
		game.CalculateOrganQuantity();
		game.SetEndRound(false);
	}
}

void SceneManager::UpdateGameOver()
{
}

void SceneManager::CheckPlayersReady()
{

}

SceneManager::SceneManager()
{
	sceneState = Scene::INIT;
}

SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	while (sceneState != Scene::GAMEOVER)
	{
		switch (sceneState)
		{
		case Scene::INIT:
			Start();
			break;
		case Scene::START:
			UpdateInit();
			break;
		case Scene::GAME:
			UpdateGame();
			break;
		}
	}
}
