#include "../include/Utils.hpp"
#include <cctype>
#include <sstream>

//according to rfc 1459 (arbitrary/most widely used), max user nickname is 9 chars
//this method ensures that it follows those regulations
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
}
