#include "../include/Server.hpp"
#include <limits>
#include <algorithm>

static void	sendGoodCommand(const Client& sender, Channel& channel, const std::string& mode, const std::string& target)
{
	std::string message = ":" + sender.getNick() + " MODE " + channel.getName() + " " + mode;
	// if (target.empty())
	// 	std::cout << "EMPTY\n";
	// else
	// 	std::cout << "NON\n";
	target.empty() ? message += "\r\n" : message += " " + target + "\r\n";
	for (size_t i = 0 ; i < channel.getClientCount() ; i++)
		send(channel.getClients()[i]->getFd(), message.c_str(), message.size(), 0);
}

void	Server::addInvite(const int& fd, char sign, const std::string& channelName)
{
	std::vector<Channel>::iterator channel = getChannel(channelName);
	if (channel != _channels.end())
	{
		if (sign == '+')
		{
			channel->setInviteMode(true);
			sendGoodCommand(*getClient(fd), *channel, "+i", "");
			std::cout << "Invite-only mode enabled for channel: " << channel->getName() << std::endl;
		}
		else
		{
			channel->setInviteMode(false);
			sendGoodCommand(*getClient(fd), *channel, "-i", "");
			std::cout << "Invite-only mode disabled for channel: " << channel->getName() << std::endl;
		}
		reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " " + sign + "i ");
	}
	else
		std::cout << "Channel not found for client." << std::endl;
}

void	Server::addTopicRestriction(const int& fd, char sign, const std::string& channelName)
{
	std::vector<Channel>::iterator channel = getChannel(channelName);
	if (channel != _channels.end())
	{
		if (sign == '+')
		{
			channel->setTopicRestriction(true);
			sendGoodCommand(*getClient(fd), *channel, "+t", "");
			std::cout << "Topic change restriction enabled for channel: " << channel->getName() << std::endl;
		}
		else
		{
			channel->setTopicRestriction(false);
			sendGoodCommand(*getClient(fd), *channel, "-t", "");
			std::cout << "Topic change restriction disabled for channel: " << channel->getName() << std::endl;
		}
		reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " " + sign + "t ");
	}
	else
		std::cout << "Channel not found for client." << std::endl;
}

void	Server::addPassword(const int& fd, char sign, const std::string& channelName, std::vector<std::string>& input)
{
	if (input.empty() || input[0].empty())
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));

	std::vector<Channel>::iterator channel = getChannel(channelName);
	if (channel != _channels.end())
	{
		if (sign == '+')
		{
			if (!channel->getPassword().empty())
				return (reply(getClient(fd), ERR_KEYSET, channelName + " :Channel key already set"));
			if (input.empty())
			{
				std::cout << "No password provided." << std::endl;
				return;
			}
			channel->setPassword(input[0]);
			sendGoodCommand(*getClient(fd), *channel, "+k", input[0]);
			reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " +k " + input[0]);
			std::cout << "Channel password has been changed for channel " << channel->getName() << " [" << input[0] << "]" << std::endl;
		}
		else
		{
			std::cout << sign << std::endl;
			channel->setPassword("");
			std::cout << "Channel password has been removed for channel " << channel->getName() << std::endl;
			sendGoodCommand(*getClient(fd), *channel, "-k", "");
			reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " -k ");
			return;
		}
	}
	else
		std::cout << "Channel not found for client." << std::endl;
}

void	Server::addOperator(char sign, const std::string& channelName, const int& fd, std::vector<std::string>& input)
{
	if (input.empty() || input[0].empty())
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));

	std::vector<Channel>::iterator channel = getChannel(channelName);
	std::vector<Client>::iterator clientIt = getClient(input[0]);
	if (clientIt == clients.end())
		return reply(getClient(fd), ERR_NOSUCHNICK, input[0] + " :No such nick/channel");
	std::vector<Client>::iterator target = getClient(input[0]);
	if (target == clients.end() || !target->isInChannel(channelName))
		return reply(getClient(fd), ERR_USERNOTINCHANNEL, input[0] + " " + channelName + " :They aren't on that channel");

	if (sign == '+')
	{
		getChannel(channelName)->addAdmin(target->getFd());
		sendGoodCommand(*getClient(fd), *channel, "+o", target->getNick());
		std::cout << input[0] << " is now an operator" << std::endl;
	}
	else
	{
		getChannel(channelName)->deleteAdmin(fd);
		sendGoodCommand(*getClient(fd), *channel, "-o", target->getNick());
		std::cout << input[0] << " is not an operator anymore" << std::endl;
	}
	reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " " + sign + "o " + input[0]);
}

void	Server::addUserLimit(const int& fd, char sign, const std::string& channelName, std::vector<std::string>& input)
{
	std::vector<Channel>::iterator channel = getChannel(channelName);
	if (sign == '+')
	{
		if (input.empty() || input[0].empty())
			return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));
		int limit;
		limit = atoi(input[0].c_str());
		if (limit > 0)
		{
			getChannel(channelName)->setClientLimit(limit);
			std::cout << "Channel has been limited to " << input[0] << " users" << std::endl;
			reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " +l " + input[0]);
			sendGoodCommand(*getClient(fd), *channel, "+l", input[0]);
		}
		else
			std::cout << "Please enter a valid number" << std::endl;
	}
	else
	{
		getChannel(channelName)->setClientLimit(-1);
		std::cout << "Channel's user limit has been removed" << std::endl;
		reply(getClient(fd), RPL_CHANNELMODEIS, channelName + " " + sign + "l ");
		sendGoodCommand(*getClient(fd), *channel, "-l", "");
	}
}

void	Server::checkModes(const int& fd, std::string str, std::vector<std::string> input, const std::string& channelName)
{
	std::vector<std::string> modifiedInput;	
	for (size_t i = 2; i < input.size(); i++)
	{
		if (input[i][0] != '+' && input[i][0] != '-')
			modifiedInput.push_back(input[i]);
	}	

	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '+' || str[i] == '-')
		{
			char sign = str[i];
			i++;
			while (i < str.length() && str[i] != '+' && str[i] != '-')
			{
				if (str[i] == 'i')
					addInvite(fd, sign, channelName);
				else if (str[i] == 't')
					addTopicRestriction(fd, sign, channelName);
				else if (str[i] == 'k')
				{
					addPassword(fd, sign, channelName, modifiedInput);
					if (!modifiedInput.empty())
						modifiedInput.erase(modifiedInput.begin());
				}
				else if (str[i] == 'o')
				{
					addOperator(sign, channelName, fd, modifiedInput);
					if (!modifiedInput.empty())
						modifiedInput.erase(modifiedInput.begin());
				}
				else if (str[i] == 'l')
				{
					addUserLimit(fd, sign, channelName, modifiedInput);
					if (!modifiedInput.empty())
						modifiedInput.erase(modifiedInput.begin());
				}
				else
					return reply(getClient(fd), ERR_UNKNOWNMODE, std::string(1, str[i]) + " :is unknown mode char to me for " + channelName);
				i++;
			}
		}
	}
}

void	Server::mode(const int& fd, const std::vector<std::string>& input)
{
	std::string channelName = input[1];
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 3)
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::vector<Channel>::iterator channel = getChannel(channelName);
	if (channel == _channels.end())
		return reply(getClient(fd), ERR_NOSUCHCHANNEL, channelName + " :No such channel");
	if (getClient(fd)->isAdmin(*getChannel(channelName)) == false)
		return (reply(getClient(fd), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"));
	if (getClient(fd) == clients.end() || !getClient(fd)->isInChannel(channelName))
		return reply(getClient(fd), ERR_USERNOTINCHANNEL, getClient(fd)->getNick() + " " + channelName + " :They aren't on that channel");
	
	for (size_t i = 1; i < input.size(); i++)
	{
		if (input[i][0] == '+' || input[i][0] == '-')
			checkModes(fd, input[i], input, channelName);
	}
}
