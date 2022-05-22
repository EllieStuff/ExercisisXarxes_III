#pragma once
#include "GameManager.h"
#include <mutex>

class SceneManager
{
	std::mutex mtx;
	enum class State {INIT, GAME, END};
	State* gameState;
	int matchID;

	std::map<int, std::map<Commands, CriticalMessages>*>* criticalMessages;

	GameManager* game;

	void SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time, int _id);

	void UpdateInit();
	void ReceiveMessages();
	void EnterGame();
	void UpdateGame();
	void CheckMessageTimeout();
	void MessageReceived(Commands message, int _id, float _rttKey);
	void SearchMatch(int _id, int _matchID, bool _createOrSearch);
	void UpdateGameInfo(int _gameID);

	void DisconnectClient(int _id);

	void ExitThread();
public:
	SceneManager();
	~SceneManager();

	void Update();
};