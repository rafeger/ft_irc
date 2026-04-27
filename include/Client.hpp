#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <unistd.h>
#include <string>
#include <iostream>

class Client
{
	private:
		int		_clientFd;
		std::string	_IPAddr;
		bool _passOK;
		bool _isRegistered;
		std::string _username;
		std::string _nickname;
		std::string _buffer;

	public:
		Client();
		~Client();

		int getFd() const;
		void setFd(int fd);
		void setIPAddr(const std::string& IPAddr);
		const std::string& getIPAddr() const;
		bool isPassOK() const;
		void setPassOK(bool value);
		bool isRegistered() const;
		void setRegistered(bool value);
		void setNickname(const std::string& nickname);
		const std::string& getNickname() const;
		void setUsername(const std::string& username);
		const std::string& getUsername() const;
		bool hasNick() const;
		bool hasUser() const;
		void appendBuffer(const std::string& data);
		const std::string& getBuffer() const;
};

#endif