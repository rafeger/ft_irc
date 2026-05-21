#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include "Client.hpp"

class Channel
{
	private:
		std::string _name;
		std::string _topic;
		std::vector<Client*> _clients;
		std::set<Client*> _operators;
		std::set<Client*> _invitedClients;
		bool _inviteOnly;
		bool _topicRestricted;
		std::string _password;
		bool _hasPassword;
		size_t _userLimit;
		bool _hasUserLimit;

	public:
		Channel(const std::string& name);
		~Channel();

		const std::string& getName() const;
		const std::string& getTopic() const;
		void setTopic(const std::string& topic);
		void addClient(Client* client);
		void removeClient(Client* client);
		bool hasClient(Client* client) const;
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool isOperator(Client* client) const;
		void setInviteOnly(bool value);
		bool isInviteOnly() const;
		void setTopicRestricted(bool value);
		bool isTopicRestricted() const;
		void setPassword(const std::string& password);
		bool checkPassword(const std::string& password) const;
		bool hasPassword() const;
		void setUserLimit(size_t limit);
		bool hasUserLimit() const;
		bool isFull() const;
		bool isEmpty() const;
		void inviteClient(Client* client);
		bool isInvited(Client* client) const;
		void removeInvite(Client* client);
		void broadcast(const std::string& msg, Client* client);
		const std::vector<Client*>& getClients() const;
		size_t getUserLimit() const;
};

#endif