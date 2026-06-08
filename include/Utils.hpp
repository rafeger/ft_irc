#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <vector>

namespace Utils
{
	bool						isValidNickname(const std::string& nick);
	std::vector<std::string>	split(const std::string& str, char delimiter);
}

#endif
