#include "ClientData.h"
#include<numeric>

void ClientData::GenerateSALT(int& value, int numOfBytes = 4)
{
	value = rand() & 0xff;
	for (size_t i = 1; i < numOfBytes; i++)
	{
		value |= (rand() & 0xff) << (i * 8);
	}
}

ClientData::ClientData(unsigned short _port, IpAddress _address, std::string _name, int _salt): port(_port), address(_address), name(_name), clientSALT(_salt)
{
	GenerateSALT(serverSALT);
}

bool ClientData::CheckChallengeResult(int _saltresult)
{
	int result = serverSALT & clientSALT;

	if (result == _saltresult)
		return true;

	return false;
}

float ClientData::GetRttAvarage()
{
	if (rttLog.size() <= 0) return 0.0f;

	return std::accumulate(rttLog.begin(), rttLog.end(), 0) / rttLog.size();
}
