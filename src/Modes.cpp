#include "../include/Server.hpp"
#include <limits>
#include <algorithm>

static void	sendGoodCommand(const Client& sender, Channel& channel, const std::string& mode, const std::string& target)
{
	std::string message = ":" + sender.getNick() + " MODE " + channel.getName() + " " + mode;
	if (target.empty())
		std::cout << "EMPTY\n";
	else
		std::cout << "NON\n";
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
	}
	else
		std::cout << "Channel not found for client." << std::endl;
}

int	Server::checkChannelPassword(const int& fd, std::string channel, const std::vector<std::string>& input)
{

	if (getChannel(channel)->getPassword().empty() == false)
	{
		// messageFromServer(fd, std::string("Please enter " + channel + "'s password\n"));	
		if (strcmp(input[0].c_str(), (getChannel(channel)->getPassword()).c_str()) != 0)
		{
			std::cout << "Wrong password" << std::endl;				std::cout << input[0] << std::endl;
			messageFromServer(fd, "Wrong password!\n");
			return -1;
		}
		else
		{
			if (std::find(getChannel(channel)->getAdmins().begin(), getChannel(channel)->getAdmins().end(), fd) != getChannel(channel)->getAdmins().end())
			    getChannel(channel)->addAdmin(fd);
			getChannel(channel)->addClient(*getClient(fd));
			std::cout << "Connected to channel " << channel << "!" << std::endl;
			messageFromServer(fd, std::string("Connected to channel " + channel + "!\n"));
		}
	}
	return 0;
}

void	Server::addPassword(const int& fd, char sign, const std::string& channelName, std::vector<std::string>& input)
{
	(void)fd;
	std::vector<Channel>::iterator channel = getChannel(channelName);
	if (channel != _channels.end())
	{
		if (sign == '+')
		{
			if (input.empty())
			{
				std::cout << "No password provided." << std::endl;
				return;
			}
			channel->setPassword(input[0]);
			std::cout << "Channel password has been changed for channel " << channel->getName() << " [" << input[0] << "]" << std::endl;
		}
		else
		{
			std::cout << sign << std::endl;
			channel->setPassword("");
			std::cout << "Channel password has been removed for channel " << channel->getName() << std::endl;
			return;
		}
	}
	else
		std::cout << "Channel not found for client." << std::endl;
}

void	Server::addOperator(char sign, const std::string& channelName, const int& fd, std::vector<std::string>& input)
{
	std::vector<Client>::iterator clientIt = getClient(input[0]);
	if (clientIt == clients.end())
		return;
	std::vector<Channel>::iterator channel = getChannel(channelName);
	std::vector<Client>::iterator target = getClient(input[3]);
	if (channel == _channels.end())
		return;
	if (sign == '+')
	{
		getChannel(channelName)->addAdmin(target->getFd());
		sendGoodCommand(*getClient(fd), *channel, "+o", target->getNick());
		std::cout << input[2] << " is now an operator" << std::endl;
	}
	else
	{
		getChannel(channelName)->deleteAdmin(fd);
		sendGoodCommand(*getClient(fd), *channel, "-o", target->getNick());
		std::cout << input[2] << " is not an operator anymore" << std::endl;
	}
}

void	Server::addUserLimit(const int& fd, char sign, const std::string& channelName, std::vector<std::string>& input)
{
	(void)fd;
	if (sign == '+')
	{
		size_t limit;
		limit = atoi(input[0].c_str());
		if (limit > 0)
		{
			getChannel(channelName)->setClientLimit(limit);
			std::cout << "Channel has been limited to " << input[0] << " users" << std::endl;
		}
		else
			std::cout << "Please enter a valid number" << std::endl;
	}
	else
	{
		getChannel(channelName)->setClientLimit(-1);
		std::cout << "Channel's user limit has been removed" << std::endl;
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
				if (str[i] == 't')
					addTopicRestriction(fd, sign, channelName);
				if (str[i] == 'k')
                		{
                			addPassword(fd, sign, channelName, modifiedInput);
                			if (!modifiedInput.empty())
                				modifiedInput.erase(modifiedInput.begin());
                		}
				if (str[i] == 'o')
                		{
					addOperator(sign, channelName, fd, input);
        				// if (!modifiedInput.empty())
        				// 	modifiedInput.erase(modifiedInput.begin());
                		}
				if (str[i] == 'l')
                		{
                			addUserLimit(fd, sign, channelName, modifiedInput);
                			if (!modifiedInput.empty())
                				modifiedInput.erase(modifiedInput.begin());
                		}
				i++;
			}
		}
	}
}

void	Server::parseModes(const int& fd, const std::vector<std::string>& input, const std::string& channelName)
{
	for (size_t i = 1; i < input.size(); i++)
	{
		if (input[i][0] == '+' || input[i][0] == '-')
			checkModes(fd, input[i], input, channelName);
	}
}

void	Server::mode(const int& fd, const std::vector<std::string>& input)
{
	std::string channelName = input[1];
	if (getClient(fd)->isAllowed() == false)
	{
		std::cout << "You have to enter the password first" << std::endl;
		return;
	}
	if (input.size() == 1)
		return;
	else
		parseModes(fd, input, channelName);
	// i: Set/remove Invite-only channel
	// t: Set/remove the restrictions of the TOPIC command to channel
	// operators
	// k: Set/remove the channel key (password)
	// o: Give/take channel operator privilege
	// l: Set/remove the user limit to channel
}