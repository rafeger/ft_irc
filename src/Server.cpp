#include "../include/Server.hpp"
#include "../include/Client.hpp"

Server::Server()
{
	_serverSocket = -1;
}

Server::~Server()
{
	closeFd();
}

bool Server::_signal = false;
void Server::handleSignal(int signum)
{
	(void)signum;
	std::cout << "Signal received!" << std::endl;
	Server::_signal = true;
}

void Server::createServerSocket()
{
	struct sockaddr_in serverAddr;
	struct pollfd serverPoll;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(this->_port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw Server::SocketException();
	int val = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
		throw Server::SetsockoptException();
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == -1)
		throw Server::FcntlException();
	if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		throw Server::BindException();
	if (listen(_serverSocket, SOMAXCONN) == -1)
		throw Server::ListenException();

	serverPoll.fd = _serverSocket;
	serverPoll.events = POLLIN;
	serverPoll.revents = 0;
	_pollfds.push_back(serverPoll);
}

void Server::closeFd()
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	if (_serverSocket != -1)
	{
		close(_serverSocket);
		_serverSocket = -1;
	}
	_clients.clear();
}

void Server::acceptClient()
{
	Client* cl = new Client();
	struct sockaddr_in clientAddr;
	struct pollfd clientPoll;
	socklen_t len = sizeof(clientAddr);
	int accClient = accept(_serverSocket, (sockaddr*)&clientAddr, &len);
	if (accClient == -1)
		throw Server::AcceptException();
	if (fcntl(accClient, F_SETFL, O_NONBLOCK) == -1)
		throw Server::FcntlException();
	clientPoll.fd = accClient;
	clientPoll.events = POLLIN;
	clientPoll.revents = 0;
	cl->setFd(accClient);
	cl->setIPAddr(inet_ntoa(clientAddr.sin_addr));
	_pollfds.push_back(clientPoll);
	_clients[accClient] = cl;
	std::cout << "Client " << accClient << " connected" << std::endl;
}

void Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		close(fd);
		delete it->second;
		_clients.erase(it);
	}
	for (size_t i = 0; i < _pollfds.size(); i++)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break ;
		}
	}
}

void Server::receivedMessage(int fd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		std::cout << "Client " << fd << " disconnected" << std::endl;
		removeClient(fd);
	}
	else
	{
		buffer[bytes] = '\0';
		std::cout << "Client " << fd << " Data: " << buffer;
	}
}

void Server::initServer()
{
	this->_port = 6667;
	createServerSocket();
	std::cout << "Server " << _serverSocket << " connected" << std::endl;

	while (_signal == false)
	{
		if ((poll(&_pollfds[0], _pollfds.size(), -1) == -1) && _signal == false)
			throw Server::PollException();
		for (size_t i = 0; i < _pollfds.size(); i++)
		{
			if (_pollfds[i].revents & POLLIN)
			{
				if (_pollfds[i].fd == _serverSocket)
					acceptClient();
				else
					receivedMessage(_pollfds[i].fd);
			}
			_pollfds[i].revents = 0;
		}
	}
	closeFd();
}
