#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include <algorithm>


//plenty of helpers functions such as getters etc to access info without cluttering 
//other methods

//also has constructors and destructors for Channel class

Channel::Channel(const std::string& name) : _name(name), _topic(""), 
_inviteOnly(false), _topicRestricted(false), _password(""), 
_hasPassword(false), _userLimit(0), _hasUserLimit(false)
{}

Channel::~Channel()
{}

const std::string& Channel::getName() const
{
	return _name;
}

const std::string& Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

void Channel::addClient(Client* client)
{
	if (!client || hasClient(client))
		return ;
	if (_hasUserLimit && _clients.size() >= _userLimit)
		return ;
	_clients.push_back(client);
	if (_clients.size() == 1)
		_operators.insert(client);
}

void Channel::removeClient(Client* client)
{
	if (!client)
		return ;
	_clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
	_operators.erase(client);
	if (_operators.empty() && !_clients.empty())
		_operators.insert(_clients.front());
}

//checks if a client is in _client
bool Channel::hasClient(Client* client) const
{
	return (std::find(_clients.begin(), _clients.end(), client) != _clients.end());
}

//maks a client in a channel an operator
void Channel::addOperator(Client *client)
{
	if (!client || !hasClient(client))
		return ;
	_operators.insert(client);
}

//remove op rights to a client in a channel
void Channel::removeOperator(Client *client)
{
	if (!client)
		return ;
	_operators.erase(client);
	if (_operators.empty() && !_clients.empty())
		_operators.insert(_clients.front());
}

//check if operator
bool Channel::isOperator(Client *client) const
{
	return (_operators.find(client) != _operators.end());
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

bool Channel::isInviteOnly() const
{
	return (_inviteOnly);
}

void Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}

bool Channel::isTopicRestricted() const
{
	return (_topicRestricted);
}

void Channel::setPassword(const std::string& password)
{
	_password = password;
	_hasPassword = true;
}

bool Channel::checkPassword(const std::string& password) const
{
	if (!_hasPassword)
		return (true);
	return (_password == password);
}

bool Channel::hasPassword() const
{
	return (_hasPassword);
}

void Channel::removePassword()
{
	_password.clear();
	_hasPassword = false;
}

void Channel::setUserLimit(size_t limit)
{
	_userLimit = limit;
	_hasUserLimit = true;
}

bool Channel::hasUserLimit() const
{
	return (_hasUserLimit);
}

//i think this isnt even callled cuz channel limit not implemented
void Channel::removeUserLimit()
{
	_userLimit = 0;
	_hasUserLimit = false;
}
//simple getter for the modes of the chan
std::string Channel::getModes() const
{
	std::string mode = "+";
	std::string param;

	if (_inviteOnly)
		mode += "i";
	if (_topicRestricted)
		mode += "t";
	if (_hasPassword)
	{
		mode += "k";
		param += " " + _password;
	}
	if (_hasUserLimit)
	{
		mode += "l";
		std::ostringstream oss;
		oss << _userLimit;
		param += " " + oss.str();
	}
	if (mode == "+")
		return (mode);
	return (mode + param);
}

bool Channel::isFull() const
{
	if (!_hasUserLimit)
		return (false);
	return (_clients.size() >= _userLimit);
}

bool Channel::isEmpty() const
{
	return (_clients.empty());
}

void Channel::inviteClient(Client* client)
{
	_invitedClients.insert(client);
}

bool Channel::isInvited(Client* client) const
{
	return (_invitedClients.find(client) != _invitedClients.end());
}

void Channel::removeInvite(Client* client)
{
	_invitedClients.erase(client);
}

void Channel::broadcast(const std::string& msg, Client *client)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i] != client)
			_clients[i]->sendMessage(msg);
	}
}

const std::vector<Client*>& Channel::getClients() const
{
	return (_clients);
}

size_t Channel::getUserLimit() const
{
	return (_userLimit);
}