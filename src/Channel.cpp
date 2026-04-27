#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <algorithm>

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
}