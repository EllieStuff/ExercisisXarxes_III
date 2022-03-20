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

	sceneState = Scene::INIT;
}

void SceneManager::EnterGame()
{
	game.ConnectP2P(serverSock, (int*)sceneState);

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
	std::cout << "Welcome!" << std::endl;
	std::cout << "1. Create P2P Game" << std::endl;
	std::cout << "2. List P2P Games" << std::endl;
	std::cout << "3. Join P2P Game" << std::endl;

	std::cout << "\nSelect option: ";

	Commands option;
	int tmpOption;
	std::cin >> tmpOption;
	option = (Commands)tmpOption;
	if (option < Commands::CREATE_GAME || option > Commands::JOIN_GAME)
		return;

	OutputMemoryStream* out = new OutputMemoryStream();
	Status status;
	out->Write(option);
	serverSock.Send(out, status);
	delete out;

	if (option == Commands::CREATE_GAME)
	{
		game.CreateGame(serverSock);
	}
	else if (option == Commands::GAME_LIST)
	{
		game.ListCurrentGames(serverSock);
	}
	else if (option == Commands::JOIN_GAME)
	{
		game.JoinGame(serverSock);
	}


	if (option == Commands::CREATE_GAME || option == Commands::JOIN_GAME) 
	{
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

SceneManager::SceneManager()
{
	sceneState = Scene::START;
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
		case Scene::START:
			Start();
			break;
		case Scene::INIT:
			UpdateInit();
			break;
		case Scene::GAME:
			UpdateGame();
			break;
		}
	}
}