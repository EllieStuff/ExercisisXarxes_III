#pragma once
#include "../res/TcpSocket.h"
#include "../res/TcpListener.h"
#include "GameManager.h"

class SceneManager
{
public:
	enum class Scene { INIT, GAME, GAMEOVER };

private:
	Scene scene;
	TcpSocket serverSock;

	GameManager game;

	void EnterInit();
	void EnterGame();

	void ExitInit();
	void ExitGame();

	void UpdateInit();
	void UpdateGame();
	void UpdateGameOver();

	void CreateGame();
	void ListCurrentGames();
	void JoinGame();

public:
	SceneManager();
	~SceneManager();

	void Update();
};