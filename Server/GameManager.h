#pragma once
#include "../res/UdpSocket.h"
#include "ClientData.h"

class GameManager
{

	unsigned short port;
	IpAddress address;
	std::string userName;
	std::map<int, ClientData*> clients;
	int currentId;
	UdpSocket sock;

public:
	GameManager();
	GameManager(std::string _address, unsigned short _port);
	~GameManager();

	void Update();
	bool ExistClient(int _id);

	void AddClientRtt(int _id, float _rtt) { clients[_id]->AddClientRtt(_rtt); }

	int GetServerSalt(int _id) { return clients[_id]->GetServerSalt(); };
	int GetClientSalt(int _id) { return clients[_id]->GetClientSalt(); };
	IpAddress GetClientAddress(int _id) { return clients[_id]->GetAddress(); };
	unsigned short GetClientPort(int _id) { return clients[_id]->GetPort(); };
	float GetClientRtt(int _id, int _rttIdx) { return clients[_id]->GetClientRtt(_rttIdx); };
	float GetClientRttAvarage(int _id) { return clients[_id]->GetRttAvarage(); }

	Status BindSocket();
	InputMemoryStream* ReceiveMSG(std::pair<IpAddress, unsigned short>* client, Status& status);

	int CreateClient(unsigned short _port, IpAddress _address, std::string _name, int _salt);
	void DeleteClient(int _id);
	Status SendClient(int _id, OutputMemoryStream* out);

};