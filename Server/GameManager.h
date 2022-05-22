#pragma once
#include "../res/UdpSocket.h"
#include "ClientData.h"

class GameManager
{

	unsigned short port;
	IpAddress address;
	std::string userName;
	std::map<int, ClientData*> waitingClients;
	std::map<int, ClientData*> connectedClients;
	int currentId;
	UdpSocket sock;

public:
	GameManager();
	GameManager(std::string _address, unsigned short _port);
	~GameManager();

	void Update();
	bool ExistClient(int _id);

	void ClientConnected(int _id);

	void SetClientRtt(int _id, float _rttKey, float _realRtt) { waitingClients[_id]->SetClientRtt(_rttKey, _realRtt); }

	void DisconnectClient(int _id);

	Status BindSocket();
	InputMemoryStream* ReceiveMSG(std::pair<IpAddress, unsigned short>* client, Status& status);

	int CreateClient(unsigned short _port, IpAddress _address, std::string _name, int _salt);
	void DeleteClient(int _id);
	Status SendClient(int _id, OutputMemoryStream* out);
	void SendAll(OutputMemoryStream* out);

	ClientData* GetClient(int _id);

	std::map<int, ClientData*> GetClientsMap() { return waitingClients; }
	ClientData* GetConnectedClient(int _id);

	std::map<int, ClientData*> GetConnectingClientsMap() { return waitingClients; }
	ClientData* GetConnectingClient(int _id);
};