#include "SceneManager.h"
#include "../res/OutputMemoryStream.h"
#include "../res/InputMemoryStream.h"

void SceneManager::EnterInit()
{
	sceneState = Scene::INIT;

	//Connect to server
	Status status = serverSock.Connect("127.0.0.1", 50000);
	if (status != Status::DONE) {
		return;
	}
	game.SetPort(serverSock.GetLocalPort());

}

void SceneManager::EnterGame()
{
	game.ConnectP2P(serverSock, (int*)sceneState);

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


	delete out;
}

void SceneManager::UpdateGame()
{
}

void SceneManager::UpdateGameOver()
{
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
	while (sceneState != Scene::GAMEOVER)
	{
		switch (sceneState)
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
