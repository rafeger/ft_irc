#include "../include/Utils.hpp"
#include <cctype>
#include <sstream>

//

namespace Utils
{
	// RFC 1459: nick = letter *8( letter / digit / "-" / "_" ), max 9 chars
	bool	isValidNickname(const std::string& nick)
	{
		if (nick.empty() || nick.length() > 9)
			return false;
		if (!std::isalpha(nick[0]))
			return false;
		for (size_t i = 1; i < nick.length(); i++)
		{
			if (!std::isalnum(nick[i]) && nick[i] != '-' && nick[i] != '_')
				return false;
		}
		return true;
	}

	// Splits "a,b,c" by ',' into {"a", "b", "c"}. Empty tokens are skipped.
	std::vector<std::string>	split(const std::string& str, char delimiter)
	{
		std::vector<std::string>	tokens;
		std::stringstream			ss(str);
		std::string					token;

		while (std::getline(ss, token, delimiter))
		{
			if (!token.empty())
				tokens.push_back(token);
		}
		return tokens;
	}
}
