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
	std::string topic;
	if (input.size() > 2)
	{
		for (size_t i = 2 ; i < input.size() ; i++)
		{
			topic += input[i];
			if (i != input.size() - 1)
				topic += ' ';
		}
	}
	if (getChannel(channelName) == _channels.end())
	{
		// if (getClient(fd)->getChannel().empty() == false)
		// 	getChannel(getClient(fd)->getChannel())->deleteClient(fd);
		getClient(fd)->setChannel(channelName);
		_channels.push_back(input.size() > 2 ? Channel(*getClient(fd), channelName, topic) : Channel(*getClient(fd), channelName));
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
		// if (getClient(fd)->getChannel().empty() == false)
		// 	getChannel(getClient(fd)->getChannel())->deleteClient(fd);
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
		if (getClient(input[1]) != clients.end())
		{
			messageFromServer(fd, "This username is already used\n");
			return;
		}
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
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
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
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
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
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
	if (getClient(fd)->getChannel().empty())
		return;
	if (input.size() == 1)
	{
		if (getChannel(getClient(fd)->getChannel())->getTopic().empty())
			messageFromServer(fd, "This channel has no topic\n");
		else
			messageFromServer(fd, "The topic of the channel " + getClient(fd)->getChannel() + " is " + getChannel(getClient(fd)->getChannel())->getTopic() + "\n");
		return;
	}
	if (!getClient(fd)->isAdmin() && getChannel(getClient(fd)->getChannel())->isTopicRestriction())
	{
		messageFromServer(fd, "You have to be admin to change the topic\n");
		return;
	}
	std::string topic;
	for (size_t i = 1 ; i < input.size() ; i++)
	{
		topic += input[i];
		if (i != input.size() - 1)
			topic += ' ';
	}
	getChannel(getClient(fd)->getChannel())->setTopic(topic);
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
	messageFromServer(getClient(input[1])->getFd(), '[' + getClient(fd)->getNick() + "] " + message + '\n');
}

void	Server::list(const int& fd)
{
	if (getClient(fd)->isAllowed() == false)
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
	messageFromServer(fd, "List of the channels on the server:\n");
	for (size_t i = 0 ; i < _channels.size() ; i++)
		messageFromServer(fd, "- " + _channels[i].getName() + " -> " + _channels[i].getTopic() + '\n');
}

void	Server::help(const int& fd)
{
	messageFromServer(fd, "List of the commands :\n- /PASS <password>\t\tEnter the server's password\n- /USER <username>\t\tEnter your username\n- /NICK <nickname>\t\tEnter your nickname\n- /JOIN <channel>\t\tCreate/Join a channel\n- /MSG <username> <message>\tSend a private message to a user\n- /MODE <mode>\t\t\tChange the mode of the current channel\n- /INVITE <username> <channel>\tInvite a user in a channel\n- /LIST\t\t\t\tDisplay the list of the channels\n- /KICK <username> <channel>\tKick a user from a channel\n- /PART\t\t\t\tQuit the current channel\n- /QUIT\t\t\t\tQuit the server\n- /TOPIC <topic>\t\tDisplay the topic of the current channel (without argument)/Change the topic of the current channel\n");}