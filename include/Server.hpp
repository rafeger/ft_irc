#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <map>
#include <vector>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

class Client;
class Channel;

class Server
{
	private:
		int				_port;
		std::string			_password;
		int				_serverSocket;
		std::map<int, Client*>		_clients;
		std::vector<struct pollfd>	_pollfds;
		std::map<std::string, Channel*>	_channels;
		static bool			_signal;

	public:
		Server();
		virtual ~Server();

		void createServerSocket();
		static void handleSignal(int signum);
		void closeFd();
		void acceptClient();
		void removeClient(int fd);
		void receivedMessage(int fd);
		void initServer(const std::string& port, const std::string& password);
		void run();
		Channel* getChannel(const std::string& name);
		void createChannel(const std::string& name);
		void removeChannel(const std::string& name);
		void handleCmd(Client* client, const std::string& msg);
		void handlePass(Client* client, const std::string& password);
		void handleNick(Client* client, const std::string& nickname);
		void handleUser(Client* client, const std::string& username);
		void handleJoin(Client* client, const std::string& channel);
		void handlePart(Client* client, const std::string& channel);
		void handlePrivateMsg(Client* client, const std::string& target, const std::string& msg);
		void tryRegister(Client* client);

		class SocketException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! socket() failed!");
				}
		};
		class SetsockoptException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! setsockopt() failed!");
				}
		};
		class FcntlException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! fcntl() failed!");
				}
		};
		class BindException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! bind() failed!");
				}
		};
		class ListenException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! listen() failed!");
				}
		};
		class AcceptException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! accept() failed!");
				}
		};
		class PollException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Error! poll() failed!");
				}
		};
};

#endif
