#pragma once
#include "../res/TcpSocket.h"
#include "../res/TcpListener.h"
#include "GameManager.h"

class SceneManager
{
public:
	enum class Scene { START, INIT, GAME, GAMEOVER };

private:
	std::mutex mtx;
	Scene sceneState;
	TcpSocket serverSock;

	GameManager game;

	void Start();
	void EnterGame();

	void ExitGame();

	void UpdateInit();
	void UpdateGame();
	void UpdateGameOver();

	void CheckPlayersReady();

public:
	SceneManager();
	~SceneManager();

	void Update();
};