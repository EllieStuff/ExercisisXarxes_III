#include "SceneManager.h"
#include <iostream>
#include <thread>

void SceneManager::EnterGame()
{

}

void SceneManager::UpdateGame()
{

}

SceneManager::SceneManager()
{
	gameState = State::INIT;
	game = new GameManager();
}

void SceneManager::UpdateInit()
{
	bool connect = false;

	game->address.SetAdress("127.0.0.1");
	game->port = 5000;

	std::cout << "Write your username" << std::endl;
	std::cin >> game->userName;

	while (!connect)
	{
		std::cout << " Connecting to the server" << std::endl;

		OutputMemoryStream* out = new OutputMemoryStream();
		out->WriteString("Hello_"+game->userName);
		Status status;
		
		game->sock.Send(out, status, *game->address.GetAddress(), game->port);

		if (status == Status::DONE)
			connect = true;
		else
			std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

SceneManager::~SceneManager()
{

}

void SceneManager::Update()
{
	while (gameState != State::END)
	{
		UpdateInit();
	}
}