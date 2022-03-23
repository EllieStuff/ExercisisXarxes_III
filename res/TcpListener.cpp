#include "TcpListener.h"
#include "TcpSocket.h"

TcpListener::TcpListener()
{
	listener.setBlocking(false);
}

TcpListener::~TcpListener()
{
}

Status TcpListener::Accept(TcpSocket &_socket)
{
	sf::TcpListener::Status status = listener.accept(_socket.socket);

	return (Status)(int)status;
}

void TcpListener::Close()
{
	listener.close();
}

Status TcpListener::Listen(unsigned short _port)
{
	sf::TcpListener::Status status = listener.listen(_port);

	return (Status)(int)status;
}

unsigned short TcpListener::GetLocalPort()
{
	return listener.getLocalPort();
}

bool TcpListener::IsBlocking()
{
	return listener.isBlocking();
}

void TcpListener::SetBlocking(bool _blocking)
{
	listener.setBlocking(_blocking);
}
