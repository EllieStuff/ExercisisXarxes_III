#pragma once
#include "../res/IpAddress.h"
#include <time.h>

class ClientData
{
	unsigned short port;
	IpAddress address;
	std::string name;
	int clientSALT, serverSALT;
	clock_t timeStamp;

	int tries;

	void GenerateSALT(int &value, int numOfBytes);

public:
	ClientData() {}
	ClientData(unsigned short _port, IpAddress _address, std::string _name);

};