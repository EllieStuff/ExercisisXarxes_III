#pragma once
#include "../res/TcpSocket.h"
#include "../res/TcpListener.h"
#include "GameManager.h"

class SceneManager
{
public:
	enum class Scene { INIT, GAME, GAMEOVER };

private:
	Scene sceneState;
	TcpSocket serverSock;

	GameManager game;

	void EnterInit();
	void EnterGame();

	void ExitGame();

	void UpdateInit();
	void UpdateGame();
	void UpdateGameOver();

public:
	SceneManager();
	~SceneManager();

	void Update();
};