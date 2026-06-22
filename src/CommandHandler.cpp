#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/IRCConstants.hpp"
#include "../include/Utils.hpp"
#include <cctype>
#include <set>

// Index-based parser — handles ":prefix CMD p1 p2 :trailing text"
// We strip a leading ':' prefix (server-to-server) if present.

//parser -> soccupe de suppr le :(qui permet de mettre des espaces)
std::vector<std::string> CommandHandler::parseCommand(const std::string& line)
{
	std::vector<std::string> tokens;
	size_t i = 0;
	const size_t len = line.size();

	// Skip a leading ':server' prefix (rare in client→server, but be safe)
	if (len > 0 && line[0] == ':')
	{
		while (i < len && line[i] != ' ')
			++i;
	}

	while (i < len)
	{
		while (i < len && line[i] == ' ')
			++i;
		if (i >= len)
			break;
		if (line[i] == ':')
		{
			tokens.push_back(line.substr(i + 1));
			break;
		}
		size_t start = i;
		while (i < len && line[i] != ' ')
			++i;
		tokens.push_back(line.substr(start, i - start));
	}
	return tokens;
}


//recupere la commande utilise par lutilisateur et appelle la methode conrrespondante
void CommandHandler::handle(Server* server, Client* client, const std::string& line)
{
	std::vector<std::string> tokens = parseCommand(line);
	if (tokens.empty())
		return;

	std::string cmd = tokens[0];
	for (size_t i = 0; i < cmd.size(); ++i)
		cmd[i] = static_cast<char>(std::toupper((unsigned char)cmd[i]));

	std::vector<std::string> params(tokens.begin() + 1, tokens.end());
	if (cmd == "CAP" || cmd == "PONG")
		return;
	if (!client->isPassOK())
	{
		if (cmd == "PASS")
			handlePass(server, client, params);
		else if (cmd == "QUIT")
			handleQuit(server, client, params);
		else
			client->sendReply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}
	if (!client->isRegistered())
	{
		if (cmd == "NICK")
			handleNick(server, client, params);
		else if (cmd == "USER")
			handleUser(server, client, params);
		else if (cmd == "QUIT")
			handleQuit(server, client, params);
		else if (cmd == "PASS")
			client->sendReply(ERR_ALREADYREGISTRED, ":You may not reregister");
		else
			client->sendReply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	//static dispatch map to call relative method
	static std::map<std::string, HandlerFunc> handlers;
	static bool initialized = false;
	if (!initialized)
	{
		handlers["NICK"]    = &handleNick;
		handlers["USER"]    = &handleUser;
		handlers["PASS"]    = &handlePass;
		handlers["JOIN"]    = &handleJoin;
		handlers["PRIVMSG"] = &handlePrivmsg;
		handlers["NOTICE"]  = &handlePrivmsg;
		handlers["PART"]    = &handlePart;
		handlers["KICK"]    = &handleKick;
		handlers["INVITE"]  = &handleInvite;
		handlers["TOPIC"]   = &handleTopic;
		handlers["MODE"]    = &handleMode;
		handlers["QUIT"]    = &handleQuit;
		handlers["PING"]    = &handlePing;
		initialized = true;
	}

	std::map<std::string, HandlerFunc>::iterator it = handlers.find(cmd);
	if (it != handlers.end())
		it->second(server, client, params);
	else
		client->sendReply("421", cmd + " :Unknown command");
}

bool CommandHandler::requireParams(Client* client, const std::vector<std::string>& params,
	size_t min, const std::string& cmd)
{
	if (params.size() < min)
	{
		client->sendReply(ERR_NEEDMOREPARAMS, cmd + " :Not enough parameters");
		return false;
	}
	return true;
}

void CommandHandler::handlePass(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (client->isPassOK())
	{
		client->sendReply(ERR_ALREADYREGISTRED, ":You may not reregister");
		return;
	}
	if (!requireParams(client, params, 1, "PASS"))
		return;
	if (params[0] != server->getPassword())
	{
		client->sendReply(ERR_PASSWDMISMATCH, ":Password incorrect");
		server->removeClient(client->getFd(), "Bad password");
		return;
	}
	client->setPassOK(true);
}

void CommandHandler::handleNick(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 1, "NICK"))
		return;
	const std::string& newNick = params[0];

	if (!Utils::isValidNickname(newNick))
	{
		client->sendReply(ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
		return;
	}

	Client* conflict = server->getClientByNickname(newNick);
	if (conflict && conflict != client)
	{
		client->sendReply(ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
		return;
	}

	std::string oldPrefix = client->getPrefix();
	client->setNickname(newNick);

	if (client->isRegistered())
	{
		std::string msg = ":" + oldPrefix + " NICK :" + newNick;
		client->sendMessage(msg);

		std::set<Client*> notified;
		notified.insert(client);
		const std::vector<Channel*>& chans = client->getChannels();
		for (size_t i = 0; i < chans.size(); ++i)
		{
			const std::vector<Client*>& members = chans[i]->getClients();
			for (size_t j = 0; j < members.size(); ++j)
			{
				if (notified.find(members[j]) == notified.end())
				{
					members[j]->sendMessage(msg);
					notified.insert(members[j]);
				}
			}
		}
	}
	else
		tryRegister(client);
}

void CommandHandler::handleUser(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	(void)server;
	if (client->isRegistered())
	{
		client->sendReply(ERR_ALREADYREGISTRED, ":You may not reregister");
		return;
	}
	if (!requireParams(client, params, 4, "USER"))
		return;
	client->setUsername(params[0]);
	tryRegister(client);
}

void CommandHandler::tryRegister(Client* client)
{
	if (!client->isPassOK() || !client->hasNick() || !client->hasUser())
		return;
	if (client->isRegistered())
		return;
	client->setRegistered(true);
	sendWelcomeMessages(client);
}

void CommandHandler::sendWelcomeMessages(Client* client)
{
	const std::string& nick = client->getNickname();
	client->sendReply(RPL_WELCOME,  ":Welcome to the IRC Network " + nick);
	client->sendReply(RPL_YOURHOST, ":Your host is localhost, running version 1.0");
	client->sendReply(RPL_CREATED,  ":This server was created today");
	client->sendReply(RPL_MYINFO,   "localhost 1.0 o itklno");
}

void CommandHandler::handlePing(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	(void)server;
	if (!requireParams(client, params, 1, "PING"))
		return;
	client->sendMessage("PONG localhost :" + params[0]);
}

void CommandHandler::handleQuit(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	std::string reason = params.empty() ? "Client quit" : params[0];
	server->removeClient(client->getFd(), reason);
}

void CommandHandler::handleJoin(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 1, "JOIN"))
		return ;
	std::string channelName = params[0];
	if (channelName.empty() || channelName[0] != '#')
	{
		client->sendReply(ERR_NOSUCHCHANNEL, channelName + " : No such channel");
		return ;
	}
	Channel* channel = server->getChannel(channelName);
	if (!channel)
	{
		server->createChannel(channelName);
		channel = server->getChannel(channelName);
	}
	if (channel->hasClient(client))
		return ;
	if (channel->isInviteOnly() && !channel->isInvited(client))
	{
		client->sendReply(ERR_INVITEONLYCHAN, channelName + ":Cannot join channel (+i)");
		return ;
	}
	std::string key;
	if (params.size() > 1)
		key = params[1];
	if (!channel->checkPassword(key))
	{
		client->sendReply(ERR_BADCHANNELKEY, channelName + ":Cannot join channel (+k)");
		return ;
	}
	if (channel->isFull())
	{
		client->sendReply(ERR_CHANNELISFULL, channelName + ":Cannot join channel (+l)");
		return ;
	}
	channel->addClient(client);
	client->joinChannel(channel);
	if (channel->isInvited(client))
		channel->removeInvite(client);
	std::string join_msg = ":" + client->getPrefix() + " JOIN :" + channelName;
	channel->broadcast(join_msg, NULL);
	if (channel->getTopic().empty())
		client->sendReply(RPL_NOTOPIC, channelName + ":No topic is set");
	else
		client->sendReply(RPL_TOPIC, channelName + ":" + channel->getTopic());

	const std::vector<Client*>& members = channel->getClients();
	std::string namesList;
	for (size_t i = 0; i < members.size(); ++i)
	{
		if (i > 0)
			namesList += " ";
		if (channel->isOperator(members[i]))
			namesList += "@";
		namesList += members[i]->getNickname();
	}
	client->sendReply(RPL_NAMREPLY, "= " + channelName + " :" + namesList);
	client->sendReply(RPL_ENDOFNAMES, channelName + " :End of /NAMES list");
}

//TALK ABOUT THIS DURING CORRECTION
//irssi receives the message u send as part_msg and reads it
//we had put a fkin : after PART. which made or whole irssi client bug.
//i still dont rlly understand.
void CommandHandler::handlePart(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 1, "PART"))
		return ;
	std::string channelName = params[0];
	Channel* channel = server->getChannel(channelName);
	if (!channel)
	{
		client->sendReply(ERR_NOSUCHCHANNEL, channelName + ":No such channel");
		return ;
	}
	if (!channel->hasClient(client))
	{
		client->sendReply(ERR_NOTONCHANNEL, channelName + ":You're not on that channel");
		return ;
	}
	std::string part_msg = ":" + client->getPrefix() + " PART " + channelName;
	if (params.size() > 1 && !params[1].empty())
		part_msg += " :" + params[1];
	channel->broadcast(part_msg, NULL);
	channel->removeClient(client);
	client->leaveChannel(channel);
	if (channel->isEmpty())
		server->removeChannel(channelName);
}

void CommandHandler::handlePrivmsg(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 2, "PRIVMSG"))
		return ;
	std::string target = params[0];
	std::string text = params[1];
	if (text.empty())
		return ;
	std::string priv_msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
	if (!target.empty() && target[0] == '#')
	{
		Channel* channel = server->getChannel(target);
		if (!channel)
		{
			client->sendReply(ERR_NOSUCHCHANNEL, target + " :No such channel");
			return ;
		}
		if (!channel->hasClient(client))
		{
			client->sendReply(ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
			return ;
		}
		channel->broadcast(priv_msg, client);
		return ;
	}
	Client* dest = server->getClientByNickname(target);
	if (!dest)
	{
		client->sendReply(ERR_NOSUCHNICK, target + " :No such nick");
		return ;
	}
	dest->sendMessage(priv_msg);
}

//tested all ifs [x] DONE
void CommandHandler::handleKick(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 2, "KICK"))
		return ;
	std::string channelName = params[0];
	std::string username = params[1];
	Channel* channel = server->getChannel(channelName);
	if (!channel)
	{
		client->sendReply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return ;
	}
	if (!channel->hasClient(client))
	{
		client->sendReply(ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return ;
	}
	if (!channel->isOperator(client))
	{
		client->sendReply(ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return ;
	}
	Client* nick = server->getClientByNickname(username);
	if (!nick)
	{
		client->sendReply(ERR_NOSUCHNICK, username + " :No such nick");
		return ;
	}
	if (!channel->hasClient(nick))
	{
		client->sendReply(ERR_USERNOTINCHANNEL, username + " " + channelName + " :They aren't on that channel");
		return ;
	}
	std::string comment = "";
	if (params.size() > 2)
		comment = params[2];
	std::string kick_msg = ":" + client->getPrefix() + " KICK " + channelName + " " + username + " : " + comment;
	channel->broadcast(kick_msg, NULL);
	channel->removeClient(nick);
	nick->leaveChannel(channel);
	if (channel->isEmpty())
		server->removeChannel(channelName);
}

void CommandHandler::handleInvite(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 2, "INVITE"))
		return ;
	std::string nickname = params[0];
	std::string channelName = params[1];
	Channel* channel = server->getChannel(channelName);
	if (!channel)
	{
		client->sendReply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return ;
	}
	if (!channel->hasClient(client))
	{
		client->sendReply(ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return ;
	}
	if (channel->isInviteOnly() && !channel->isOperator(client))
	{
		client->sendReply(ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return ;
	}
	Client* nick = server->getClientByNickname(nickname);
	if (!nick)
	{
		client->sendReply(ERR_NOSUCHNICK, nickname + " :No such nick");
		return ;
	}
	if (channel->hasClient(nick))
	{
		client->sendReply(ERR_USERONCHANNEL, nickname + " " + channelName + " :is already on channel");
		return ;
	}
	channel->inviteClient(nick);
	client->sendReply(RPL_INVITING, nickname + " " + channelName);
	nick->sendMessage(":" + client->getPrefix() + " INVITE " + nickname + " " + channelName);
}

void CommandHandler::handleTopic(Server* server, Client* client,
	const std::vector<std::string>& params)
{
	if (!requireParams(client, params, 1, "TOPIC"))
		return ;
	std::string channelName = params[0];
	Channel* channel = server->getChannel(channelName);
	if (!channel)
	{
		client->sendReply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return ;
	}
	if (!channel->hasClient(client))
	{
		client->sendReply(ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return ;
	}
	if (params.size() == 1)
	{
		if (channel->getTopic().empty())
			client->sendReply(RPL_NOTOPIC, channelName + " :No topic is set");
		else
			client->sendReply(RPL_TOPIC, channelName + " :" + channel->getTopic());
	}
	else
	{
		if (channel->isTopicRestricted() && !channel->isOperator(client))
		{
			client->sendReply(ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
			return ;
		}
		channel->setTopic(params[1]);
		std::string topicMsg = ":" + client->getPrefix() + " TOPIC " + channelName + " :" + params[1];
		channel->broadcast(topicMsg, NULL);
	}
}
//
//case +i [x]
//case -i [x]
//case 
void CommandHandler::handleMode(Server* server, Client* client,
	const std::vector<std::string> &params)
{
	if (!requireParams(client, params, 1, "MODE"))
		return ;
	std::string target = params[0];
	if (target[0] != '#')
		return ;

	Channel* channel = server->getChannel(target);
	if (!channel)
		return (client->sendReply(ERR_NOSUCHCHANNEL, target + " :No such channel"));
	if (params.size() == 1)
		return (client->sendReply(RPL_CHANNELMODEIS, target + " " + channel->getModes()));
	if (!channel->hasClient(client))
		return (client->sendReply(ERR_NOTONCHANNEL, target + " :You're not on that channel"));
	if (!channel->isOperator(client))
		return (client->sendReply(ERR_CHANOPRIVSNEEDED, target + " :You're not channel operator"));

	const std::string& modeStr = params[1];
	size_t paramIdx = 2;
	bool adding = true;
	std::string appliedModes;
	std::string appliedParams;
	char lastSign = 0;

	for (size_t i = 0; i < modeStr.size(); ++i)
	{
		char c = modeStr[i];
		if (c == '+') { adding = true; continue; }
		if (c == '-') { adding = false; continue; }

		bool applied = false;
		std::string modeParam;

		switch (c)
		{
			case 'i':
				channel->setInviteOnly(adding);
				applied = true;
				break ;
			case 't':
				channel->setTopicRestricted(adding);
				applied = true;
				break ;
			case 'k':
				if (adding)
				{
					if (paramIdx >= params.size()) continue;
					modeParam = params[paramIdx++];
					channel->setPassword(modeParam);
				}
				else
					channel->removePassword();
				applied = true;
				break ;
			case 'l':
				if (adding)
				{
					if (paramIdx >= params.size()) continue;
					const std::string& s = params[paramIdx++];
					int n = std::atoi(s.c_str());
					if (n <= 0) continue;
					channel->setUserLimit((size_t)n);
					modeParam = s;
				}
				else
					channel->removeUserLimit();
				applied = true;
				break ;
			case 'o':
				if (paramIdx >= params.size()) continue;
				{
					const std::string& targetNick = params[paramIdx++];
					Client* targetClient = server->getClientByNickname(targetNick);
					if (!targetClient || !channel->hasClient(targetClient)) continue;
					if (adding)
						channel->addOperator(targetClient);
					else
						channel->removeOperator(targetClient);
					modeParam = targetNick;
				}
				applied = true;
				break ;
			default:
				break ;
		}
		if (applied)
		{
			char sign = adding ? '+' : '-';
			if (sign != lastSign)
			{
				appliedModes += sign;
				lastSign = sign;
			}
			appliedModes += c;
			if (!modeParam.empty())
				appliedParams += " " + modeParam;
		}
	}
	if (!appliedModes.empty())
	{
		std::string modeMsg = ":" + client->getPrefix() + " MODE " + target + " " + appliedModes + appliedParams;
		channel->broadcast(modeMsg, NULL);
	}
}
