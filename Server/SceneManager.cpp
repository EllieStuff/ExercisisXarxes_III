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
	Status status;
	status = game->BindSocket();

	criticalMessages = new std::map<int, std::map<Commands, CriticalMessages>*>();

	if(status != Status::DONE) 
	{
		std::cout << "ERROR!!" << std::endl;
		gameState = State::END;
	}

	std::thread clientReceive(&SceneManager::ReceiveMessages, this);
	clientReceive.detach();
}

void SceneManager::MessageReceived(Commands _message, int _id)
{
	auto clientPosition = criticalMessages->find(_id);
	if (clientPosition == criticalMessages->end()) return;

	auto mapPosition = clientPosition->second->find(_message);
	if (mapPosition != clientPosition->second->end())
	{
		delete mapPosition->second.message;
		clientPosition->second->erase(mapPosition);
	}
}

void SceneManager::CheckMessageTimeout()
{
	while (true)
	{
		if (criticalMessages->size() == 0) continue;
		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		Status status;
		for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
		{
			if (it->second->size() == 0) continue;
			for (auto it2 = it->second->begin(); it2 != it->second->end(); it2++)
			{
				float time = currentTime - it2->second.startTime;
				if (time > 1)
				{
					game->SendClient(it->first, it2->second.message);
					it2->second.startTime = currentTime;
				}
			}
		}
	}
}

void SceneManager::SavePacketToTable(Commands _packetId, OutputMemoryStream* out, std::time_t time, int _id)
{
	auto clientPos = criticalMessages->find(_id);

	if (clientPos == criticalMessages->end()) return;

	CriticalMessages message = CriticalMessages(game->GetClientAddress(_id), game->GetClientPort(_id), time, out);

	auto mapPosition = clientPos->second->find(_packetId);

	if (mapPosition != clientPos->second->end())
	{
		mapPosition->second = message;
	}
	else
	{
		clientPos->second->insert(std::pair<Commands, CriticalMessages>(_packetId, message));
	}
}

void SceneManager::UpdateInit() 
{

}

void SceneManager::ReceiveMessages()
{
	while (gameState != State::END)
	{
		Status status = Status::NOT_READY;

		std::pair<IpAddress, unsigned short> _client ;

		std::string ip = "127.0.0.1";
		_client.first = ip;

		InputMemoryStream* message = game->ReceiveMSG(&_client, status);

		if(status == Status::DONE) 
		{
			Commands commandNum;
			int num;

			message->Read(&num);

			commandNum = (Commands) num;

			switch (commandNum)
			{
			case Commands::WELCOME:
				break;
			case Commands::HELLO: 
				{
					OutputMemoryStream* out = new OutputMemoryStream();
					std::string name = message->ReadString();
					int salt;
					message->Read(&salt);

					int id = game->CreateClient(_client.second, _client.first, name, salt);
					out->Write(Commands::CHALLENGE);
					out->Write(id);
					out->Write(game->GetServerSalt(id));

					criticalMessages->insert(std::pair<int, std::map<Commands, CriticalMessages>*>(id, new std::map<Commands, CriticalMessages>()));

					auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

					game->SendClient(id, out);
					SavePacketToTable(commandNum, out, currentTime, id);
				}
				break;
			case Commands::PLAYER_ID:
				break;
			case Commands::SALT:
				{
					int id;
					int salt;
					message->Read(&id);
					message->Read(&salt);

					MessageReceived(Commands::SALT, id);
					MessageReceived(Commands::CHALLENGE, id);

					OutputMemoryStream* out = new OutputMemoryStream();

					int _result = game->GetServerSalt(id) & game->GetClientSalt(id);
					
					std::cout << "Server SALT: " << game->GetServerSalt(id) << ", Client SALT: " << game->GetClientSalt(id) << ", Result: " << _result << std::endl;

					if(salt == _result)
					{
						out->Write((int) Commands::WELCOME);
						out->WriteString("Welcome!");
					}
					else 
					{
						out->Write((int) Commands::SALT);
					}

					auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

					game->SendClient(id, out);
					SavePacketToTable(commandNum, out, currentTime, id);
				}
				break;
			case Commands::CHALLENGE:
				break;
			}
		}
		else 
		{
			delete message;
		}
	}
}

SceneManager::~SceneManager()
{
	for (auto it = criticalMessages->begin(); it != criticalMessages->end(); it++)
	{
		delete it->second;
	}
	delete criticalMessages;
}

void SceneManager::Update()
{
	std::thread tCheck(&SceneManager::CheckMessageTimeout, this);
	tCheck.detach();

	while (gameState != State::END)
	{
		ReceiveMessages();
	}
}