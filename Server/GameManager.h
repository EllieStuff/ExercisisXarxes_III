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

	int GetSalt(int _id) { return clients[_id]->GetSalt(); };

	Status BindSocket();
	Status ReceiveMSG(InputMemoryStream* in, std::pair<IpAddress, unsigned short> client);

	int CreateClient(unsigned short _port, IpAddress _address, std::string _name, int _salt);
	Status SendClient(int _id, OutputMemoryStream* out);

};