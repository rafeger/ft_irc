#include "../include/Server.hpp"
#include "../include/Client.hpp"

//beginning of loop ykhow it goes
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << BOLDRED <<  "Error! Enter the port and the password again!" << DEFAULT << std::endl;
        return (1);
    }
    Server serv;
    std::cout << CYAN << "WELCOME TO OUR IRC BY rafeger and thhuynh !" << DEFAULT << std::endl;
    std::cout << "                        .,,uod8B8bou,,." << std::endl;
    std::cout << "               ..,uod8BBBBBBBBBBBBBBBBRPFT?l!i:." << std::endl;
    std::cout << "          ,=m8BBBBBBBBBBBBBBBRPFT?!||||||||||||||" << std::endl;
    std::cout << "          !...:!TVBBBRPFT||||||||||!!^^\"\"'   ||||" << std::endl;
    std::cout << "          !.......:!?|||||!!^^\"\"'            ||||" << std::endl;
    std::cout << "          !.........||||                     ||||" << std::endl;
    std::cout << "          !.........||||  ##irc              ||||" << std::endl;
    std::cout << "          !.........||||   @rafeger          ||||" << std::endl;
    std::cout << "          !.........||||   @thhuynh          ||||" << std::endl;
    std::cout << "          !.........||||                     ||||" << std::endl;
    std::cout << "          !.........||||                     ||||" << std::endl;
    std::cout << "          `.........||||                    ,||||" << std::endl;
    std::cout << "           .;.......||||               _.-!!|||||" << std::endl;
    std::cout << "    .,uodWBBBBb.....||||       _.-!!|||||||||!:'" << std::endl;
    std::cout << "  !YBBBBBBBBBBBBBBb..!|||:..-!!|||||||!iof68BBBBBb...." << std::endl;
    std::cout << "  !..YBBBBBBBBBBBBBBb!!||||||||!iof68BBBBBBRPFT?!::   `." << std::endl;
    std::cout << "  !....YBBBBBBBBBBBBBBbaaitf68BBBBBBRPFT?!:::::::::     `." << std::endl;
    std::cout << "  !......YBBBBBBBBBBBBBBBBBBBRPFT?!::::::;:!^\"`;:::       `." << std::endl;
    std::cout << "  !........YBBBBBBBBBBRPFT?!::::::::::^''...::::::;         iBBbo." << std::endl;
    std::cout << "  `..........YBRPFT?!::::::::::::::::::::::::;iof68bo.      WBBBBbo." << std::endl;
    std::cout << "    `..........:::::::::::::::::::::::;iof688888888888b.     `YBBBP^'" << std::endl;
    std::cout << "      `........::::::::::::::::;iof688888888888888888888b.     `" << std::endl;
    std::cout << "        `......:::::::::;iof688888888888888888888888888888b." << std::endl;
    std::cout << "          `....:::;iof688888888888888888888888888888888899fT!" << std::endl;
    std::cout << "            `..::!8888888888888888888888888888888899fT|!^\"'" << std::endl;
    std::cout << "              `' !!988888888888888888888888899fT|!^\"'" << std::endl;
    std::cout << "                `!!8888888888888888899fT|!^\"'" << std::endl;
    std::cout << "                  `!988888888899fT|!^\"'" << std::endl;
    std::cout << "                    `!9899fT|!^\"'" << std::endl;
    std::cout << "                      `!^\"'" << std::endl;
    std::cout << "" << std::endl;
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