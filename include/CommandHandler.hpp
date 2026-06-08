#ifndef COMMANDHANDLER_HPP
# define COMMANDHANDLER_HPP

# include <string>
# include <vector>
# include <map>

class Server;
class Client;

class CommandHandler
{
	public:
		static void	handle(Server* server, Client* client, const std::string& line);

	private:
		typedef void (*HandlerFunc)(Server*, Client*, const std::vector<std::string>&);

		static std::vector<std::string>	parseCommand(const std::string& line);
		static void						tryRegister(Client* client);
		static void						sendWelcomeMessages(Client* client);

		// --- Connection lifecycle (Person A) ---
		static void	handlePass(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleNick(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleUser(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handlePing(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleQuit(Server* server, Client* client, const std::vector<std::string>& params);

		// --- Channel commands (Person B) ---
		static void	handleJoin(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handlePart(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handlePrivmsg(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleKick(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleInvite(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleTopic(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleMode(Server* server, Client* client, const std::vector<std::string>& params);
};

#endif
