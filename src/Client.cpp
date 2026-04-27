#include "../include/Client.hpp"

Client::Client() : _clientFd(-1), _IPAddr(""), _passOK(false), _isRegistered(false)
{}

Client::~Client()
{}

int Client::getFd() const
{
	return (_clientFd);
}

void Client::setFd(int fd)
{
	_clientFd = fd;
}

void Client::setIPAddr(const std::string& IPAddr)
{
	_IPAddr = IPAddr;
}

const std::string& Client::getIPAddr() const
{
	return _IPAddr;
}

bool Client::isPassOK() const
{
	return _passOK;
}

void Client::setPassOK(bool value)
{
	_passOK = value;
}

bool Client::isRegistered() const
{
	return _isRegistered;
}

void Client::setRegistered() const
{
	_isRegistered = value;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

const std::string& Client::getUsername() const
{
	return _username;
}

bool Client::hasNick() const
{
	return !_nickname.empty();
}

bool Client::hasUser() const
{
	return !_username.empty();
}

void Client::appendBuffer(const std::string& data)
{
	_buffer += data;
}

const std::string& Client::getBuffer() const
{
	return _buffer;
}
