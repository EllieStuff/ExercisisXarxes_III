#include "IpAddress.h"

IpAddress::IpAddress()
{
}

IpAddress::IpAddress(sf::IpAddress _address)
{
	address = _address;
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
