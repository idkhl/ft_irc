#include "../include/Server.hpp"
#include <limits>
#include <algorithm>

// int Server::checkChannelPassword(const int& fd, std::string channel, const std::vector<std::string>& input)
// {
//     std::string password;
//     if (getChannel(channel)->getPassword().empty() == false)
//     {
//         messageFromServer(fd, std::string("Please enter " + channel + "'s password\n"));

//         if (strcmp(password.c_str(), (getChannel(channel)->getPassword()).c_str()) != 0)
//         {
//             std::cout << "Wrong password" << std::endl;
// 			std::cout << password << std::endl;
//             messageFromServer(fd, "Wrong password!\n");
//             return -1;
//         }
//     }
//     return 0;
// }

void	Server::addInvite(const int& fd)
{
    std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
    if (channel != _channels.end())
    {
        channel->setInviteMode(true);
        std::cout << "Invite-only mode enabled for channel: " << channel->getName() << std::endl;
    }
    else
    {
        std::cout << "Channel not found for client." << std::endl;
    }
}

void	Server::addTopicRestriction(const int& fd)
{
	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
    if (channel != _channels.end())
    {
        channel->setTopicRestriction(true);
        std::cout << "Topic change restriction enabled for channel: " << channel->getName() << std::endl;
    }
    else
    {
        std::cout << "Channel not found for client." << std::endl;
    }
}

void	Server::addPassword(const int& fd, std::vector<std::string>& input)
{
    if (input.empty())
    {
        std::cout << "No password provided." << std::endl;
        return;
    }

	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
    if (channel != _channels.end())
    {
        channel->setPassword(input[0]);
        std::cout << "Channel password has been changed for channel " << channel->getName() << " (" << input[0] << ")" << std::endl;
    }
    else
    {
        std::cout << "Channel not found for client." << std::endl;
    }
	for (size_t i = 1; i < input.size(); i++)
    {
        if (input[i][0] != '+' && input[i][0] != '-')
        {
            input.push_back(input[i]);
        }
    }
	(void)fd;
}

void	Server::addOperator(std::vector<std::string>& input)
{
    getClient(input[0])->setAdmin(true);
    std::cout << input[0] << " is now an operator" << std::endl;
}

void	Server::addUserLimit(const int& fd, std::vector<std::string>& input)
{
	size_t limit;
	limit = atoi(input[0].c_str());
	getChannel(getClient(fd)->getChannel())->setClientLimit(limit);
    std::cout << "Channel has been limited to " << input[0] << " users" << std::endl;
}

void	Server::checkModes(const int& fd, std::string str, const std::vector<std::string> input)
{
    std::vector<std::string> modifiedInput;

    for (size_t i = 1; i < input.size(); i++)
    {
        if (input[i][0] != '+' && input[i][0] != '-')
        {
            modifiedInput.push_back(input[i]);
        }
    }

    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] == '+')
        {
            i++;
            while (i < str.length() && str[i] != '+' && str[i] != '-')
            {
				std::cout << "Modified Input: ";
				for (std::vector<std::string>::iterator it = modifiedInput.begin(); it != modifiedInput.end(); ++it)
				{
					std::cout << *it << " ";
				}
				std::cout << std::endl;
                if (str[i] == 'i')
                    addInvite(fd);
                if (str[i] == 't')
                    addTopicRestriction(fd);
                if (str[i] == 'k')
                {
                    addPassword(fd, modifiedInput);
                    modifiedInput.erase(modifiedInput.begin());
                }
				if (str[i] == 'o')
                {
					addOperator(modifiedInput);
                    modifiedInput.erase(modifiedInput.begin());
                }
				if (str[i] == 'l')
                {
					addUserLimit(fd, modifiedInput);
                    modifiedInput.erase(modifiedInput.begin());
                }
                i++;
            }
        }
    }
}

void	Server::parseModes(const int& fd, const std::vector<std::string>& input)
{
	for (size_t i = 1; i < input.size(); i++)
	{
		if (input[i][0] == '+' || input[i][0] == '-')
			checkModes(fd, input[i], input);
	}
}

void	Server::mode(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isAllowed() == false)
	{
		std::cout << "You have to enter the password first" << std::endl;
		return;
	}
	if (input.size() == 1)
		return;
	else
		parseModes(fd, input);
	// i: Set/remove Invite-only channel
	// t: Set/remove the restrictions of the TOPIC command to channel
	// operators
	// k: Set/remove the channel key (password)
	// o: Give/take channel operator privilege
	// l: Set/remove the user limit to channel
}