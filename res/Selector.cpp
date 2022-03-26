#include "Selector.h"

Selector::Selector()
{
}

Selector::~Selector()
{
}

void Selector::Add(TcpSocket* sock)
{
	selector.add(*sock->GetSocket());
}

void Selector::Add(TcpListener* listener)
{
	selector.add(*listener->GetListener());
}

void Selector::Remove(TcpSocket* sock)
{
	selector.remove(*sock->GetSocket());
}

void Selector::Remove(TcpListener* listener)
{
	selector.remove(*listener->GetListener());
}

void Selector::Clear()
{
	selector.clear();
}

bool Selector::Wait()
{
	return selector.wait();
}

bool Selector::IsReady(TcpSocket* sock)
{
	return selector.isReady(*sock->GetSocket());;
}

bool Selector::IsReady(TcpListener* listener)
{
	return selector.isReady(*listener->GetListener());
}
