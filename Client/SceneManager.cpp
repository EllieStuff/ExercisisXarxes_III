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

	sceneState = new int(1);
}

void SceneManager::EnterGame()
{
	game.Start();

	std::cout << "Waiting For your turn" << std::endl;

	sceneState = new int(2);

	while(true)
	game.Update();
}


void SceneManager::ExitGame()
{
	sceneState = new int(3);

}

void SceneManager::Ready() 
{
	std::cout << "Waiting for players" << std::endl;

	while (game.GetPlayersNum() < 3) {}

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

void SceneManager::UpdateInit()
{
	std::cout << "\nWelcome!" << std::endl;
	std::cout << "1. Create P2P Game" << std::endl;
	std::cout << "2. List P2P Games" << std::endl;
	std::cout << "3. Join P2P Game" << std::endl;

	std::cout << "\nSelect option: ";

	Commands option;
	char tmpOption;
	
	std::cin >> tmpOption;
	tmpOption -= '0';
	option = (Commands)tmpOption;

	if (option < Commands::CREATE_GAME || option > Commands::JOIN_GAME)
		return;

	OutputMemoryStream* out = new OutputMemoryStream();
	Status status;
	out->Write(option);
	serverSock.Send(out, status);
	delete out;

	//mtx.lock();
	if (option == Commands::CREATE_GAME)
	{
		game.CreateGame(&serverSock);
	}
	else if (option == Commands::GAME_LIST)
	{
		game.ListCurrentGames(&serverSock);
	}
	else if (option == Commands::JOIN_GAME)
	{
		game.JoinGame(&serverSock);
	}
	//mtx.unlock();

	if (option == Commands::CREATE_GAME || option == Commands::JOIN_GAME) {
		std::thread t(&SceneManager::Ready, this);
		t.detach();
		game.ConnectP2P(&serverSock, sceneState);
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
	sceneState = new int(0);
}

SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	sceneState = new int(0);
	while (*sceneState != 3)
	{
		/*system("cls");*/
		switch (*sceneState)
		{
		case 0:
			Start();
			break;
		case 1:
			UpdateInit();
			break;
		case 2:
			UpdateGame();
			break;
		}
	}
}
