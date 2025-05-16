#include "../include/Server.hpp"

void	Server::addInvite(const std::string& channelName)
{
	std::vector<Channel>::iterator channel = getChannel(channelName);
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

void	Server::addTopicRestriction(const std::string& channelName)
{
	std::vector<Channel>::iterator channel = getChannel(channelName);
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

// void	Server::addPassword(const int& fd, std::vector<std::string>& input)
// {
//     if (input.empty())
//     {
//         std::cout << "No password provided." << std::endl;
//         return;
//     }

// 	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
//     if (channel != _channels.end())
//     {
//         channel->setPassword(input[0]);
//         std::cout << "Channel password has been changed" << channel->getName() << std::endl;
//     }
//     else
//     {
//         std::cout << "Channel not found for client." << std::endl;
//     }
// 	(void)fd;
// }

void	Server::checkModes(std::string str, const std::vector<std::string> input, const std::string& channelName)
{
	std::vector<std::string> modifiedInput;	
	for (size_t i = 1; i < input.size(); i++)
	{
		if (input[i][0] != '+' && input[i][0] != '-')
		{
			modifiedInput.push_back(input[i]);
		}
	}	
	std::cout << "Modified Input: ";
	for (std::vector<std::string>::iterator it = modifiedInput.begin(); it != modifiedInput.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '+')
		{
			i++;
			while (i < str.length() && str[i] != '+' && str[i] != '-')
			{
				if (str[i] == 'i')
					addInvite(channelName);
				if (str[i] == 't')
					addTopicRestriction(channelName);
				// if (str[i] == 'k')
				//	addPassword(fd, modifiedInput);
				i++;
			}
		}
	}
}

void	Server::parseModes(const std::vector<std::string>& input, const std::string& channelName)
{
	for (size_t i = 1; i < input.size(); i++)
	{
		if (input[i][0] == '+' || input[i][0] == '-')
			checkModes(input[i], input, channelName);
	}
}