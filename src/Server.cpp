#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/CommandHandler.hpp"

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
	_clients.clear();
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();
	if (_serverSocket != -1)
	{
		close(_serverSocket);
		_serverSocket = -1;
	}
}

//lookup what is that
void Server::acceptClient()
{
	Client* cl = new Client();
	struct sockaddr_in clientAddr;
	struct pollfd clientPoll;
	socklen_t len = sizeof(clientAddr);
	int accClient = accept(_serverSocket, (sockaddr*)&clientAddr, &len);
	if (accClient == -1)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			throw Server::AcceptException();
		delete cl;
		return;
	}
	if (fcntl(accClient, F_SETFL, O_NONBLOCK) == -1)
		throw Server::FcntlException();
	clientPoll.fd = accClient;
	clientPoll.events = POLLIN;
	clientPoll.revents = 0;
	cl->setFd(accClient);
	cl->setHostname(inet_ntoa(clientAddr.sin_addr));
	_pollfds.push_back(clientPoll);
	_clients[accClient] = cl;
	std::cout << "Client " << accClient << " connected" << std::endl;
}

// Broadcasts QUIT to all of the client's channels, then removes and deletes them.
void Server::removeClient(int fd, const std::string& reason)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;
	Client* client = it->second;

	std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + reason;
	std::vector<Channel*> clientChans = client->getChannels(); // copy — removeClient modifies original
	for (size_t i = 0; i < clientChans.size(); ++i)
	{
		clientChans[i]->broadcast(quitMsg, client);
		clientChans[i]->removeClient(client);
		if (clientChans[i]->isEmpty())
		{
			std::string chanName = clientChans[i]->getName();
			removeChannel(chanName);
		}
	}

	// RFC 1459: server sends ERROR before closing — irssi waits for this
	std::string errMsg = ":localhost ERROR :Closing Link: " + client->getPrefix() + " (" + reason + ")\r\n";
	send(fd, errMsg.c_str(), errMsg.size(), MSG_NOSIGNAL | MSG_DONTWAIT);

	std::cout << "Client " << fd << " disconnected (" << reason << ")" << std::endl;
	close(fd);
	delete client;
	_clients.erase(it);
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}
}

void Server::receivedMessage(int fd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes == 0)
	{
		std::cout << "Client " << fd << " disconnected" << std::endl;
		removeClient(fd, "Connection closed");
		return;
	}
	if (bytes < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			removeClient(fd, "Read error");
		return;
	}
	buffer[bytes] = '\0';

	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;
	Client* client = it->second;
	client->appendBuffer(buffer);

	std::string& recvBuf = client->getBuffer();
	size_t pos;
	while ((pos = recvBuf.find('\n')) != std::string::npos)
	{
		std::string line = recvBuf.substr(0, pos);
		recvBuf.erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.resize(line.size() - 1);
		CommandHandler::handle(this, client, line);
		if (_clients.find(fd) == _clients.end())
			return;
	}
}

//
void Server::initServer(const std::string& port, const std::string& password)
{
	this->_port = atoi(port.c_str());
	this->_password = password;

	signal(SIGPIPE, SIG_IGN);
	createServerSocket();
	std::cout << "Server listening on port " << _port << std::endl;

	while (_signal == false)
	{
		if ((poll(&_pollfds[0], _pollfds.size(), -1) == -1) && _signal == false)
			throw Server::PollException();

		for (int i = static_cast<int>(_pollfds.size()) - 1; i >= 0; --i)
		{
			bool isServerFd = (_pollfds[i].fd == _serverSocket);
			int  fd         = _pollfds[i].fd;

			// POLLIN first: read any pending data before acting on hangup.
			// irssi sends QUIT then closes immediately, so both POLLIN and
			// POLLHUP can be set in the same poll() — we must process the
			// data first or the QUIT message is silently discarded.
			if (_pollfds[i].revents & POLLIN)
			{
				if (isServerFd)
					acceptClient();
				else
					receivedMessage(fd);
			}

			// Error / hangup — only after we have drained the read buffer
			if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				if (!isServerFd && _clients.find(fd) != _clients.end())
					removeClient(fd, "Connection error");
				continue;
			}

			if (!isServerFd)
			{
				// receivedMessage may have removed this client (QUIT, bad password)
				std::map<int, Client*>::iterator cit = _clients.find(fd);
				if (cit == _clients.end())
					continue;

				if (_pollfds[i].revents & POLLOUT)
					cit->second->trySend();

				// Keep POLLOUT set only while there is data waiting to go out
				if (cit->second->hasPendingData())
					_pollfds[i].events |= POLLOUT;
				else
					_pollfds[i].events &= ~POLLOUT;
			}

			_pollfds[i].revents = 0;
		}
	}
}

//getter called in handle join method to provide a channel name or user
Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return it->second;
	return NULL;
}

//is called when  uer creates a new chan by being the first to join it
void Server::createChannel(const std::string& name)
{
	_channels[name] = new Channel(name);
}

//deletes a channel (is called automatically when chan is empty)
void Server::removeChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
	{
		delete it->second;
		_channels.erase(it);
	}
}

Client* Server::getClientByNickname(const std::string& nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}
