#ifndef COMMANDHANDLER_HPP
# define COMMANDHANDLER_HPP

# include <string>
# include <vector>
# include <map>

class Server;
class Client;


//cette structure est la pour 'dispatch' les differentes commandes appelles.
//on utilise une dispatch map (une methode privee) pour gerer les differents commandes
//utilisees par lutilisateur.
// -> pros -> on evite dutiliser 50k if else tout moches
// -cons -> jai mis 3 jours a comprendre
class CommandHandler
{
	public:
		static void	handle(Server* server, Client* client, const std::string& line);

	private:
		typedef void (*HandlerFunc)(Server*, Client*, const std::vector<std::string>&);

		static std::vector<std::string>	parseCommand(const std::string& line);
		static void						tryRegister(Client* client);
		static void						sendWelcomeMessages(Client* client);
		static bool						requireParams(Client* client, const std::vector<std::string>& params, size_t min, const std::string& cmd);

		//connection
		static void	handlePass(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleNick(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleUser(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handlePing(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleQuit(Server* server, Client* client, const std::vector<std::string>& params);

		// chan commands
		static void	handleJoin(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handlePart(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handlePrivmsg(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleKick(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleInvite(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleTopic(Server* server, Client* client, const std::vector<std::string>& params);
		static void	handleMode(Server* server, Client* client, const std::vector<std::string>& params);
};

#endif
