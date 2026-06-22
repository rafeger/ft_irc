#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>

class Channel;

class Client
{
	private:
		int						_fd;
		std::string				_hostname;
		std::string				_nickname;
		std::string				_username;
		std::string				_recvBuffer;
		std::string				_sendBuffer;
		bool					_passOK;
		bool					_registered;
		std::vector<Channel*>	_channels;

	public:
		Client();
		~Client();

		// Getters
		int							getFd() const;
		const std::string&			getHostname() const;
		const std::string&			getNickname() const;
		const std::string&			getUsername() const;
		std::string&				getBuffer();
		const std::string&			getBuffer() const;
		const std::vector<Channel*>& getChannels() const;
		std::string					getPrefix() const;

		// Setters
		void	setFd(int fd);
		void	setHostname(const std::string& hostname);
		void	setNickname(const std::string& nickname);
		void	setUsername(const std::string& username);
		void	setPassOK(bool value);
		void	setRegistered(bool value);

		// State checks
		bool	isPassOK() const;
		bool	isRegistered() const;
		bool	hasNick() const;
		bool	hasUser() const;
		bool	hasPendingData() const;
		bool	isInChannel(Channel* channel) const;

		// Buffering
		void	appendBuffer(const std::string& data);

		// Messaging
		void	sendMessage(const std::string& msg);
		void	sendReply(const std::string& code, const std::string& message);
		bool	trySend();

		// Channel membership tracking
		void	joinChannel(Channel* channel);
		void	leaveChannel(Channel* channel);
};

#endif
