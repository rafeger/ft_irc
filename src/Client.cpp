#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include <sys/socket.h>
#include <algorithm>

Client::Client() : _fd(-1), _hostname(""), _passOK(false), _registered(false)
{}

Client::~Client()
{}

// --- Getters ---

int						Client::getFd()			const { return _fd; }
const std::string&		Client::getHostname()	const { return _hostname; }
const std::string&		Client::getNickname()	const { return _nickname; }
const std::string&		Client::getUsername()	const { return _username; }
const std::string&		Client::getRealname()	const { return _realname; }
std::string&			Client::getBuffer()			  { return _recvBuffer; }
const std::string&		Client::getBuffer()		const { return _recvBuffer; }

const std::vector<Channel*>& Client::getChannels() const { return _channels; }

// nick!user@host — used as the sender prefix in all outgoing IRC messages
std::string Client::getPrefix() const
{
	return _nickname + "!" + _username + "@" + _hostname;
}

// --- Setters ---

void	Client::setFd(int fd)							{ _fd = fd; }
void	Client::setHostname(const std::string& h)		{ _hostname = h; }
void	Client::setNickname(const std::string& nick)	{ _nickname = nick; }
void	Client::setUsername(const std::string& user)	{ _username = user; }
void	Client::setRealname(const std::string& real)	{ _realname = real; }
void	Client::setPassOK(bool value)					{ _passOK = value; }
void	Client::setRegistered(bool value)				{ _registered = value; }

// --- State checks ---

bool	Client::isPassOK()		const { return _passOK; }
bool	Client::isRegistered()	const { return _registered; }
bool	Client::hasNick()		const { return !_nickname.empty(); }
bool	Client::hasUser()		const { return !_username.empty(); }
bool	Client::hasPendingData() const { return !_sendBuffer.empty(); }

bool	Client::isInChannel(Channel* channel) const
{
	return std::find(_channels.begin(), _channels.end(), channel) != _channels.end();
}

// --- Buffering ---

void	Client::appendBuffer(const std::string& data)
{
	_recvBuffer += data;
}

// --- Messaging ---

// Queues a message for sending — actual I/O happens in trySend() via POLLOUT
void	Client::sendMessage(const std::string& msg)
{
	_sendBuffer += msg + "\r\n";
}

// Formats a numeric reply and queues it: ":localhost <code> <nick> <message>"
void	Client::sendReply(const std::string& code, const std::string& message)
{
	std::string nick = _nickname.empty() ? "*" : _nickname;
	sendMessage(":localhost " + code + " " + nick + " " + message);
}

// Flushes as much of _sendBuffer as the socket accepts right now (non-blocking)
bool	Client::trySend()
{
	if (_sendBuffer.empty())
		return true;

	#ifdef __APPLE__
		ssize_t sent = send(_fd, _sendBuffer.c_str(), _sendBuffer.length(), MSG_DONTWAIT);
	#else
		ssize_t sent = send(_fd, _sendBuffer.c_str(), _sendBuffer.length(), MSG_DONTWAIT | MSG_NOSIGNAL);
	#endif

	if (sent > 0)
		_sendBuffer.erase(0, sent);
	return _sendBuffer.empty();
}

// --- Channel membership tracking ---

void	Client::joinChannel(Channel* channel)
{
	if (!isInChannel(channel))
		_channels.push_back(channel);
}

void	Client::leaveChannel(Channel* channel)
{
	std::vector<Channel*>::iterator it = std::find(_channels.begin(), _channels.end(), channel);
	if (it != _channels.end())
		_channels.erase(it);
}
