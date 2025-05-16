#include "../include/Server.hpp"

std::string	Server::join(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return "";
	}
	if (input.size() == 1)
		return "";
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
		getClient(fd)->addChannel(channelName);
		_channels.push_back(input.size() > 2 ? Channel(*getClient(fd), channelName, topic) : Channel(*getClient(fd), channelName));
		std::cout << "Channel " << channelName << " created!" << std::endl;
		messageFromServer(fd, std::string("Channel " + channelName + " created!\n"));
	}
	else
	{
		if (getChannel(channelName)->isInviteOnly() && !getClient(fd)->isInvitedIn(channelName))
		{
			messageFromServer(fd, "You can not enter this channel because you are not invited\n");
			return "";
		}
		if (getChannel(channelName)->getClientLimit() > 0 && getChannel(channelName)->getClientCount() == getChannel(channelName)->getClientLimit())
		{
			messageFromServer(fd, "Channel's client limit has been reached\n");
			return "";
		}
		getChannel(channelName)->addClient(*getClient(fd));
		std::cout << "Connected to channel " << channelName << "!" << std::endl;
		messageFromServer(fd, std::string("Connected to channel " + channelName + "!\n"));
	}
	return channelName;
}

void	Server::quit(const int& fd)
{
	ClearClients(fd);
}

void	Server::pass(const int& fd, const std::vector<std::string>& input)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client == clients.end())
		return;
	if (client->getUser().empty())
	{
		messageFromServer(fd, "You have to enter your username first (/user <username>)\n");
		return;
	}
	if (client->getNick().empty())
	{
		messageFromServer(fd, "You have to enter your nickname first (/nick <nickname>)\n");
		return;
	}
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
		getClient(fd)->setConnexion(true);
		std::string response = ":localhost 001 " + getClient(fd)->getNick() + " :Welcome to the IRC server\r\n"
							":localhost 002 " + getClient(fd)->getNick() + " :Your host is localhost\r\n"
							":localhost 003 " + getClient(fd)->getNick() + " :This server was created today\r\n";
		send(fd, response.c_str(), response.length(), 0);
	}
}

void	Server::user(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return;
	std::vector<Client>::iterator client = getClient(fd);
	if (client != clients.end())
	{
		if (client->getUser().empty() == false && client->isConnected())
		{
			messageFromServer(fd, "You already have a username: " + client->getUser() + '\n');
			return;
		}
		if (getClient(input[1]) != clients.end())
		{
			messageFromServer(fd, "This username is already used\n");
			return;
		}
		client->setUser(input[1]);
		std::cout << "User name set to " << input[1] << std::endl;
		messageFromServer(fd, std::string("User name set to " + input[1] + '\n'));
	}
}

void	Server::nick(const int& fd, const std::vector<std::string>& input)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client == clients.end())
		return;
	if (client->getUser().empty())
	{
		messageFromServer(fd, "You have to enter your username first (/user <username>)\n");
		return;
	}
	if (input.size() == 1)
		return;
	if (client != clients.end())
	{
		client->setNick(input[1]);
		messageFromServer(fd, "Nick name set to " + input[1] + "\n");
		std::cout << "Nick name set to " << input[1] << std::endl;
	}
}

void	Server::kick(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 3)
		return;
	std::string channelName = input[1];
	if (getChannel(channelName) == _channels.end())
	{
		messageFromServer(fd, "There is no channel named " + channelName + '\n');
		return;
	}
 	if (getClient(fd)->isAdmin(*getChannel(channelName)))
	{
		messageFromServer(fd, "You do not have the right to kick someone\n");
		return;
	}
	for (size_t i = 2 ; i < input.size() ; i++)
	{
		if (getClient(input[i]) == clients.end() || getClient(input[i])->isInChannel(channelName))
			messageFromServer(fd, "There is no user named " + input[i] + " in your channel\n");
		else
		{
			getClient(input[i])->deleteChannel(channelName);
			getChannel(channelName)->deleteClient(getClient(input[i])->getFd());
			getChannel(channelName)->deleteAdmin(getClient(input[i])->getFd());
			messageFromServer(fd, "You kicked " + input[i] + " form the channel " + channelName + "\n");
		}
	}
}

void	Server::invite(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 3)
		return;
	std::string channelName = input[1];
	if (getClient(fd)->isAdmin(*getChannel(channelName)) == false)
	{
		messageFromServer(fd, "You do not have the right to invite someone\n");
		return;
	}
	for (size_t i = 2 ; i < input.size() ; i++)
	{
		if (getClient(input[i]) == clients.end())
			messageFromServer(fd, "There is no user named " + input[i] + "\n");
		else
		{
			getClient(input[i])->addInvitation(channelName);
			messageFromServer(fd, "You invited " + input[i] + " to the channel " + channelName + "\n");
		}
	}
}

void	Server::topic(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 2)
		return;
	std::string channelName = input[1];
	if (getChannel(channelName) == _channels.end())
	{
		messageFromServer(fd, "There is no channel named " + channelName + '\n');
		return;
	}
	if (input.size() == 2)
	{
		if (getChannel(channelName)->getTopic().empty())
			messageFromServer(fd, "This channel has no topic\n");
		else
			messageFromServer(fd, "The topic of the channel " + channelName + " is " + getChannel(channelName)->getTopic() + "\n");
		return;
	}
	if (!getClient(fd)->isAdmin(*getChannel(channelName)) && getChannel(channelName)->isTopicRestriction())
	{
		messageFromServer(fd, "You have to be an operator to change the topic\n");
		return;
	}
	std::string topic;
	for (size_t i = 2 ; i < input.size() ; i++)
	{
		topic += input[i];
		if (i != input.size() - 1)
			topic += ' ';
	}
	getChannel(channelName)->setTopic(topic);
}

void	Server::msg(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() <= 2)
		return;
	if (getClient(input[1]) == clients.end() || getClient(input[1])->isConnected() == false)
	{
		messageFromServer(fd, "There is no user named " + input[1] + " connected to the server\n");
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
	messageFromServer(fd, "List of the channels on the server:\n");
	for (size_t i = 0 ; i < _channels.size() ; i++)
		messageFromServer(fd, "- " + _channels[i].getName() + " -> " + _channels[i].getTopic() + '\n');
}

void	Server::help(const int& fd)
{
	messageFromServer(fd, "List of the commands :\n- /PASS <password>\t\tEnter the server's password\n- /USER <username>\t\tEnter your username\n- /NICK <nickname>\t\tEnter your nickname\n- /JOIN <channel>\t\tCreate/Join a channel\n- /MSG <username> <message>\tSend a private message to a user\n- /MODE <mode>\t\t\tChange the mode of the current channel\n- /INVITE <username> <channel>\tInvite a user in a channel\n- /LIST\t\t\t\tDisplay the list of the channels\n- /KICK <username> <channel>\tKick a user from a channel\n- /PART\t\t\t\tQuit the current channel\n- /QUIT\t\t\t\tQuit the server\n- /TOPIC <topic>\t\tDisplay the topic of the current channel (without argument)/Change the topic of the current channel\n");
}

void	Server::mode(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	std::string channelName = input[1];
	if (getClient(fd)->isAdmin(*getChannel(channelName)) == false)
	{
		messageFromServer(fd, "You have to be an operator to use /MODE\n");
		return;
	}
	if (getChannel(channelName) == _channels.end())
	{
		messageFromServer(fd, "There is no channel named " + channelName + '\n');
		return;
	}
	if (input.size() < 2)
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