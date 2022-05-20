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

	std::map<int, ClientData*> GetClientsMap() { return waitingClients; }

	int GetServerSalt(int _id) { return waitingClients[_id]->GetServerSalt(); };
	int GetClientSalt(int _id) { return waitingClients[_id]->GetClientSalt(); };
	IpAddress GetClientAddress(int _id) { return waitingClients[_id]->GetAddress(); };
	unsigned short GetClientPort(int _id) { return waitingClients[_id]->GetPort(); };
	float GetClientRtt(int _id, float _rttKey) { return waitingClients[_id]->GetPort(); };

	Status BindSocket();
	InputMemoryStream* ReceiveMSG(std::pair<IpAddress, unsigned short>* client, Status& status);

	int CreateClient(unsigned short _port, IpAddress _address, std::string _name, int _salt);
	void DeleteClient(int _id);
	Status SendClient(int _id, OutputMemoryStream* out);

};