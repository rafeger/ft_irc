#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"

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
		return ;
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

void Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return ;
	Client* client = it->second;
	for (std::map<std::string, Channel*>::iterator ch = _channels.begin(); ch != _channels.end();)
	{
		ch->second->removeClient(client);
		if (ch->second->isEmpty())
		{
			delete ch->second;
			_channels.erase(ch);
		}
		else
			++ch;
	}
	close(fd);
	delete client;
	_clients.erase(it);
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
	if (bytes == 0)
	{
		std::cout << "Client " << fd << " disconnected" << std::endl;
		removeClient(fd);
		return ;
	}
	else if (bytes < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			removeClient(fd);
		return ;
	}
	buffer[bytes] = '\0';
	Client* client = _clients[fd];
	client->appendBuffer(buffer);
	std::cout << "Client " << fd << " Data: " << buffer;
	std::string& buf = client->getBuffer();
	size_t pos;
	while ((pos = buf.find("\r\n")) != std::string::npos)
	{
		std::string line = buf.substr(0, pos);
		buf.erase(0, pos + 2);
		handleCmd(client, line);
	}
}

void Server::initServer(const std::string& port, const std::string& password)
{
	this->_port = atoi(port.c_str());
	this->_password = password;

	createServerSocket();
	std::cout << "Server " << _serverSocket << " connected" << std::endl;

	while (_signal == false)
	{
		if ((poll(&_pollfds[0], _pollfds.size(), -1) == -1) && _signal == false)
			throw Server::PollException();
		for (int i = static_cast<int>(_pollfds.size()) - 1; i >= 0; --i)
		{
			if (_pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				removeClient(_pollfds[i].fd);
				continue ;
			}
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
}

void Server::handleJoin(Client *client, const std::string& channel)
{
	if (!client)
		return ;
	if (channel.empty() || channel[0] != '#')
	{
		client->sendMessage("403 " + channel + " :No such channel");
		return ;
	}
	Channel* chan = getChannel(channel);
	if (!chan)
	{
		createChannel(channel);
		chan = getChannel(channel);
	}
	if (chan->isInviteOnly() && !chan->isInvited(client))
	{
		client->sendMessage("473 " + channel + " :Cannot join channel (+i)");
		return ;
	}
	if (chan->isFull())
	{
		client->sendMessage("471 " + channel + " :Cannot join channel (+l)");
		return ;
	}
	if (chan->hasClient(client))
		return ;
	chan->addClient(client);
	std::string msg = ":" + client->getPrefix() + " JOIN " + channel;
	chan->broadcast(msg, NULL);
	client->sendMessage(msg);
	if (!chan->getTopic().empty())
		client->sendMessage("332 " + client->getNickname() + " " + channel + " :" + chan->getTopic());
	else
		client->sendMessage("331 " + client->getNickname() + " " + channel + ":No topic is set");
}

void Server::handlePass(Client *client, const std::string& password)
{
	if (!client)
		return ;
	if (client->isRegistered())
	{
		client->sendMessage("462 :You may not register");
		return ;
	}
	if (password != _password)
	{
		client->sendMessage("464 :Password incorrect");
		removeClient(client->getFd());
		return ;
	}
	client->setPassOK(true);
}

void Server::handleNick(Client* client, const std::string& nickname)
{
	if (!client)
		return ;
	if (nickname.empty())
	{
		client->sendMessage("431 :No nickname given");
		return ;
	}
	if (client->getNickname() == nickname)
	{
		client->sendMessage("433 " + nickname + " :Nickname is already in use");
		return ;
	}
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
		{
			client->sendMessage("433 " + nickname + " :Nickname is already in use");
			return ;
		}
	}
	bool wasRegistered = client->isRegistered();
	std::string oldNickname = client->getNickname();
	client->setNickname(nickname);
	tryRegister(client);
	if (wasRegistered)
	{
		std::string msg = ":" + oldNickname + " NICK :" + nickname + "\r\n";
		for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			if (it->second->hasClient(client))
				it->second->broadcast(msg, NULL);
		}
		client->sendMessage(msg);
	}
}

void Server::handleUser(Client* client, const std::string& username)
{
	if (!client)
		return ;
	if (client->isRegistered())
	{
		client->sendMessage("462 :You may not register");
		return ;
	}
	if (username.empty())
	{
		client->sendMessage("461 USER :Not enough parameters");
		return ;
	}
	client->setUsername(username);
	tryRegister(client);
}

void Server::tryRegister(Client* client)
{
	if (!client->isPassOK())
		return ;
	if (!client->hasNick() || !client->hasUser())
		return ;
	if (client->isRegistered())
		return ;
	client->setRegistered(true);
	client->sendMessage("001 :Welcome to the IRC Server");
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return (it->second);
	return (NULL);
}

void Server::createChannel(const std::string& name)
{
	_channels[name] = new Channel(name);
}

void Server::removeChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
	{
		delete it->second;
		_channels.erase(it);
	}
}

void Server::handleCmd(Client* client, const std::string& msg)
{
	std::istringstream iss(msg);
	std::string cmd, arg;

	iss >> cmd >> arg;

	if (cmd == "PASS")
		handlePass(client, arg);
	else if (cmd == "USER")
	{
		std::string username = arg;
		handleUser(client, username);
	}
	else if (cmd == "JOIN")
	{
		if (!client->isRegistered())
		{
			client->sendMessage("451 :You have not registered");
			return ;
		}
		handleJoin(client, arg);
	}
}