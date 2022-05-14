#include "IpAddress.h"

IpAddress::IpAddress()
{
}

IpAddress::IpAddress(sf::IpAddress _address)
{
	address = _address;
}

IpAddress::IpAddress(std::string _address)
{
	address = sf::IpAddress(_address);
}

IpAddress::~IpAddress()
{
}

void IpAddress::SetAddress(std::string _address)
{
	address = sf::IpAddress(_address);
}

std::string IpAddress::GetLocalAddress()
{
	return address.getLocalAddress().toString();
}

std::string IpAddress::GetPublicAddress()
{
	return address.getPublicAddress().toString();
}
