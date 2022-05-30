#pragma once
#include "../res/UdpSocket.h"
#include "ClientData.h"

class GameManager
{
	ClientData* client = nullptr;
	UdpSocket sock;

	void BindPort(unsigned short& _OutPort);
public:
	GameManager();
	GameManager(std::string _address, unsigned short _port);
	~GameManager();

	void Update();

	void SafeSend(OutputMemoryBitStream* out);

	UdpSocket* GetSocket() { return &sock; }

	void InitClient(std::string _name, std::string _address);

	IpAddress GetAddress() { return client->GetAddress(); }

	int GetClientID() { return client->GetClientID(); };
	int GetServerSalt() { return client->GetServerSalt(); };
	int GetClientSalt() { return client->GetClientSalt(); };

	void SetServerSalt(int _salt) { client->SetServerSalt(_salt); }
	void SetClientSalt(int _salt) { client->SetClientSalt(_salt); }

	void SetClientID(int _id) { client->SetClientID(_id); }

	unsigned short GetPort() { return client->GetPort(); }
	std::string GetName() { return client->GetName(); }
};