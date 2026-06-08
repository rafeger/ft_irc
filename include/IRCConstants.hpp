#ifndef IRCCONSTANTS_HPP
# define IRCCONSTANTS_HPP

//what is this file u may ask. HAHA

//so basically we are preprocessing *not compiling* those info beforehead, so no extern dec needed etc, no 
//compil time since #define is handled b4r compilation hihi


//The IRC protocol uses 3 digits numbers as basically the language the server uses to talks with the client(s).

//so every information (error, connection whatever) is a number, as shown below !
//more here !

//https://www.alien.net.au/irc/irc2numerics.html



// --- Successful replies ---
# define RPL_WELCOME         "001"
# define RPL_YOURHOST        "002"
# define RPL_CREATED         "003"
# define RPL_MYINFO          "004"

// --- Channel/user info replies ---
# define RPL_UMODEIS         "221"
# define RPL_CHANNELMODEIS   "324"
# define RPL_NOTOPIC         "331"
# define RPL_TOPIC           "332"
# define RPL_INVITING        "341"
# define RPL_NAMREPLY        "353"
# define RPL_ENDOFNAMES      "366"

// --- Error replies ---
# define ERR_NOSUCHNICK      "401"
# define ERR_NOSUCHCHANNEL   "403"
# define ERR_CANNOTSENDTOCHAN "404"
# define ERR_ERRONEUSNICKNAME "432"
# define ERR_NICKNAMEINUSE   "433"
# define ERR_USERNOTINCHANNEL "441"
# define ERR_NOTONCHANNEL    "442"
# define ERR_USERONCHANNEL   "443"
# define ERR_NOTREGISTERED   "451"
# define ERR_NEEDMOREPARAMS  "461"
# define ERR_ALREADYREGISTRED "462"
# define ERR_PASSWDMISMATCH  "464"
# define ERR_CHANNELISFULL   "471"
# define ERR_INVITEONLYCHAN  "473"
# define ERR_BADCHANNELKEY   "475"
# define ERR_CHANOPRIVSNEEDED "482"

#endif
