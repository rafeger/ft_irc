#include "../include/Server.hpp"
#include "../include/Client.hpp"

int main()
{
	/*if (argc != 3)
	{
		std::cout << "Error! Enter the port and the password again!" << std::endl;
		return (1);
	}*/
	Server serv;
	std::cout << "--SERVER--" << std::endl;
	try
	{
		struct sigaction sa;
		sa.sa_handler = Server::handleSignal;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		serv.initServer();
	}
	catch (const std::exception& e)
	{
		serv.closeFd();
		std::cout << e.what() << std::endl;
	}
	std::cout << "Server closed!" << std::endl;
}
