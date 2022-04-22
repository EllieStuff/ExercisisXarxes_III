#include "IpAddress.h"

IpAddress::IpAddress()
{
}

IpAddress::~IpAddress()
{
}

std::string IpAddress::GetLocalAddress()
{
	return address.getLocalAddress().toString();
}

std::string IpAddress::GetPublicAddress()
{
	return address.getPublicAddress().toString();
}
