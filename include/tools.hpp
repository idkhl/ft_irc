#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

__attribute__((always_inline))
inline std::vector<std::string>	splitInput(std::string str)
{
	bool flag = false;
	for (size_t i = 0 ; i < str.size() ; i++)
	{
		if (str[i] == '\n')
			str[i] = ' ';
	}
	for (size_t i = 0 ; i < str.size() ; i++)
	{
		if (str[i] != ' ')
			flag = true;
		if (str[i] == ' ' && flag)
			str[i] = 0;
	}
	std::vector<std::string> result;
	for (size_t i = 0 ; i < str.size() ; i++)
	{
		if (i == 0 && str[i])
			result.push_back(std::string(&str[i]));
		else if (str[i] && !str[i - 1])
			result.push_back(std::string(&str[i]));
	}
	return result;
}