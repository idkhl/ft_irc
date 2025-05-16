#include "../include/Server.hpp"
#include <limits>
#include <algorithm>

void	Server::addInvite(char sign, const int& fd)
{
	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
	if (channel != _channels.end())
	{
        if (sign == '+')
        {
            channel->setInviteMode(true);
            std::cout << "Invite-only mode enabled for channel: " << channel->getName() << std::endl;
        }
        else
        {
            channel->setInviteMode(false);
            std::cout << "Invite-only mode disabled for channel: " << channel->getName() << std::endl;
        }
	}
	else
	{
		std::cout << "Channel not found for client." << std::endl;
	}
}

void	Server::addTopicRestriction(char sign, const int& fd)
{
	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
	if (channel != _channels.end())
	{
        if (sign == '+')
        {
            channel->setTopicRestriction(true);
            std::cout << "Topic change restriction enabled for channel: " << channel->getName() << std::endl;
        }
        else
        {
            channel->setTopicRestriction(false);
            std::cout << "Topic change restriction disabled for channel: " << channel->getName() << std::endl;
        }
	}
	else
	{
		std::cout << "Channel not found for client." << std::endl;
	}
}

int Server::checkChannelPassword(const int& fd, std::string channel, const std::vector<std::string>& input)
{

    if (getChannel(channel)->getPassword().empty() == false)
    {
        // messageFromServer(fd, std::string("Please enter " + channel + "'s password\n"));

        if (strcmp(input[0].c_str(), (getChannel(channel)->getPassword()).c_str()) != 0)
        {
            std::cout << "Wrong password" << std::endl;
			std::cout << input[0] << std::endl;
            messageFromServer(fd, "Wrong password!\n");
            return -1;
        }
        else
        {
            getClient(fd)->setChannel(channel);
            if (std::find(getChannel(channel)->getAdmins().begin(), getChannel(channel)->getAdmins().end(), fd) != getChannel(channel)->getAdmins().end())
                getClient(fd)->setAdmin(true);
            getChannel(channel)->join(*getClient(fd));
            std::cout << "Connected to channel " << channel << "!" << std::endl;
            messageFromServer(fd, std::string("Connected to channel " + channel + "!\n"));
        }
    }
    return 0;
}

void	Server::addPassword(char sign, const int& fd, std::vector<std::string>& input)
{
    
    std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
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
    {
        std::cout << "Channel not found for client." << std::endl;
    }
}

void	Server::addOperator(char sign, const int& fd, std::vector<std::string>& input)
{
    std::vector<Client>::iterator clientIt = getClient(input[0]);
    if (clientIt == clients.end())
        return;
    Client* client = &(*clientIt);
    int opFd = client->getFd();
    std::string channelName = client->getChannel();
    std::vector<Channel>::iterator channel = getChannel(channelName);
    if (channel == _channels.end())
        return;
    if (sign == '+')
    {
        client->setAdmin(true);
        channel->addAdminFd(opFd);
        std::cout << input[0] << " is now an operator" << std::endl;
    }
    else
    {
        client->setAdmin(false);
        channel->removeAdminFd(opFd);
        std::cout << input[0] << " is not an operator anymore" << std::endl;
    }
    (void)fd;
}

void	Server::addUserLimit(char sign, const int& fd, std::vector<std::string>& input)
{
    if (sign == '+')
    {
        size_t limit;
        limit = atoi(input[0].c_str());
        if (limit > 0)
        {
            getChannel(getClient(fd)->getChannel())->setClientLimit(limit);
            std::cout << "Channel has been limited to " << input[0] << " users" << std::endl;
        }
        else
        {
            std::cout << "Please enter a valid number" << std::endl;
        }
    }
    else
    {
        getChannel(getClient(fd)->getChannel())->setClientLimit(-1);
        std::cout << "Channel's user limit has been removed" << std::endl;
    }
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
        if (str[i] == '+' || str[i] == '-')
        {
            char sign = str[i];
            i++;
            while (i < str.length() && str[i] != '+' && str[i] != '-')
            {
                if (str[i] == 'i')
                    addInvite(sign, fd);
                if (str[i] == 't')
                    addTopicRestriction(sign, fd);
                if (str[i] == 'k')
                {
                    addPassword(sign, fd, modifiedInput);
                    if (!modifiedInput.empty())
                        modifiedInput.erase(modifiedInput.begin());
                }
				if (str[i] == 'o')
                {
					addOperator(sign, fd, modifiedInput);
                    if (!modifiedInput.empty())
                        modifiedInput.erase(modifiedInput.begin());
                }
				if (str[i] == 'l')
                {
                    addUserLimit(sign, fd, modifiedInput);
                    if (!modifiedInput.empty())
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