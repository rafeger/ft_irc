#include "../include/Server.hpp"
#include "../include/Client.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Error! Enter the port and the password again!" << std::endl;
		return (1);
	}
	Server serv;
	std::cout << "Server initialized on port " << argv[1] << std::endl;
	std::cout << "     -------------------   " << std::endl;
	try
	{
		struct sigaction sa;
		sa.sa_handler = Server::handleSignal;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		serv.initServer(argv[1], argv[2]);
	}
	catch (const std::exception& e)
	{
		serv.closeFd();
		std::cout << e.what() << std::endl;
	}
	std::cout << "Server closed!" << std::endl;
}