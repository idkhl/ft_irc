#include "../include/Server.hpp"

void	Server::part(const int& fd)
{
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, user and nickname first\n");
		return;
	}
	if (getClient(fd)->getChannel().empty())
		return;
	getChannel(getClient(fd)->getChannel())->deleteClient(fd);
	messageFromServer(fd, std::string("You left the channel " + getClient(fd)->getChannel() + "\n"));
}

void	Server::join(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
	if (input.size() == 1)
		return;
	std::string channelName = input[1][0] == '#' ? input[1] : '#' + input[1];
	if (getChannel(channelName) == _channels.end())
	{
		if (getClient(fd)->getChannel().empty() == false)
			getChannel(getClient(fd)->getChannel())->deleteClient(fd);
		getClient(fd)->setChannel(channelName);
		_channels.push_back(Channel(*getClient(fd), channelName));
		std::cout << "Channel " << channelName << " created!" << std::endl;
		messageFromServer(fd, std::string("Channel " + channelName + " created!\n"));
	}
	else
	{
		if (getChannel(channelName)->isInviteOnly() && !getClient(fd)->isInvitedIn(channelName))
		{
			messageFromServer(fd, "You can not enter this channel because you are not invited\n");
			return;
		}
		if (getChannel(channelName)->getClientLimit() && getChannel(channelName)->getClientCount() == getChannel(channelName)->getClientLimit())
		{
			messageFromServer(fd, "Channel's client limit has been reached\n");
			return;
		}
		if (getClient(fd)->getChannel().empty() == false)
			getChannel(getClient(fd)->getChannel())->deleteClient(fd);
		getClient(fd)->setChannel(channelName);
		if (std::find(getChannel(channelName)->getAdmins().begin(), getChannel(channelName)->getAdmins().end(), fd) != getChannel(channelName)->getAdmins().end())
			getClient(fd)->setAdmin(true);
		getChannel(channelName)->join(*getClient(fd));
		std::cout << "Connected to channel " << channelName << "!" << std::endl;
		messageFromServer(fd, std::string("Connected to channel " + channelName + "!\n"));
	}
}

void	Server::quit(const int& fd)
{
	ClearClients(fd);
}

void	Server::pass(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return;
	if (strcmp(input[1].c_str(), Mdp))
	{
		std::cout << "Wrong password" << std::endl;
		messageFromServer(fd, "Wrong password!\n");
	}
	else
	{
		std::cout << "Good password" << std::endl;
		messageFromServer(fd, "You are connected!\n");
		getClient(fd)->setConnexion(true);
		if (!getClient(fd)->getUser().empty() && !getClient(fd)->getNick().empty() && getClient(fd)->isConnected())
			getClient(fd)->setAuthorization(true);
	}
}

void	Server::user(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return;
	std::vector<Client>::iterator client = getClient(fd);
	if (client != clients.end())
	{
		client->setUser(input[1]);
		std::cout << "User name set to " << input[1] << std::endl;
		messageFromServer(fd, std::string("User name set to " + input[1] + '\n'));
	}
	if (!getClient(fd)->getUser().empty() && !getClient(fd)->getNick().empty() && getClient(fd)->isConnected())
		getClient(fd)->setAuthorization(true);
}

void	Server::nick(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return;
	std::vector<Client>::iterator client = getClient(fd); 
	if (client != clients.end())
	{
		client->setNick(input[1]);
		messageFromServer(fd, "Nick name set to " + input[1] + "\n");
		std::cout << "Nick name set to " << input[1] << std::endl;
	}
	if (!getClient(fd)->getUser().empty() && !getClient(fd)->getNick().empty() && getClient(fd)->isConnected())
		getClient(fd)->setAuthorization(true);
}

void	Server::kick(const int& fd, const std::vector<std::string>& usersToKick)
{
	if (usersToKick.size() == 1)
		return;
	if (getClient(fd)->isAdmin() == false)
	{
		messageFromServer(fd, "You do not have the right to kick someone\n");
		return;
	}
	for (size_t i = 1 ; i < usersToKick.size() ; i++)
	{
		if (getClient(usersToKick[i]) == clients.end() || getClient(usersToKick[i])->getChannel() != getClient(fd)->getChannel())
			messageFromServer(fd, "There is no user named " + usersToKick[i] + " in your channel\n");
		else
		{
			part(getClient(usersToKick[i])->getFd());
			messageFromServer(fd, "You kicked " + usersToKick[i] + " form the channel " + getClient(fd)->getChannel() + "\n");
		}
	}
}

void	Server::invite(const int& fd, const std::vector<std::string>& usersToInvite)
{
	if (usersToInvite.size() == 1)
		return;
	if (getClient(fd)->isAdmin() == false)
	{
		messageFromServer(fd, "You do not have the right to invite someone\n");
		return;
	}
	for (size_t i = 1 ; i < usersToInvite.size() ; i++)
	{
		if (getClient(usersToInvite[i]) == clients.end())
			messageFromServer(fd, "There is no user named " + usersToInvite[i] + "\n");
		else
		{
			getClient(usersToInvite[i])->addInvitation(getClient(fd)->getChannel());
			messageFromServer(fd, "You invited " + usersToInvite[i] + " to the channel " + getClient(fd)->getChannel() + "\n");
		}
	}
}

void	Server::topic(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->getChannel().empty())
		return;
	if (input.size() == 1)
	{
		messageFromServer(fd, "The topic of the channel " + getClient(fd)->getChannel() + " is " + getChannel(getClient(fd)->getChannel())->getTopic() + "\n");
		return;
	}
	if (!getClient(fd)->isAdmin() && getChannel(getClient(fd)->getChannel())->isTopicRestriction())
	{
		messageFromServer(fd, "You have to be an operator to change the topic\n");
		return;
	}
	getChannel(getClient(fd)->getChannel())->setTopic(input[1]);
}

void	Server::msg(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
	if (input.size() <= 2)
		return;
	if (getClient(input[1]) == clients.end())
	{
		messageFromServer(fd, "There is no user named " + input[1] + '\n');
		return;
	}
	std::string message;
	for (size_t i = 2 ; i < input.size() ; i++)
	{
		message += input[i];
		if (i != input.size() - 1)
			message += ' ';
	}
	send(getClient(input[1])->getFd(), message.c_str(), message.size(), 0);
}