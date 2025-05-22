#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Client.hpp"

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
			reply(fd, ERR_INVITEONLYCHAN, channelName + " :Cannot join channel (+i)");
			return "";
		}
		if (getChannel(channelName)->getClientLimit() > 0 && getChannel(channelName)->getClientCount() == getChannel(channelName)->getClientLimit())
		{
			reply(fd, ERR_CHANNELISFULL, channelName + " :Cannot join channel (+l)");
			return "";
		}
		if (getChannel(channelName)->getPassword().empty() == false)
		{
			if (input.size() != 3)
			{
				reply(fd, ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
				return "";
			}
			if (strcmp(input[2].c_str(), (getChannel(channelName)->getPassword()).c_str()) != 0)
			{
				std::cout << "Wrong password" << std::endl;
				reply(fd, ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
				return "";
			}
		}
		getChannel(channelName)->addClient(*getClient(fd));
		std::cout << "Connected to channel " << channelName << "!" << std::endl;
		messageFromServer(fd, std::string("Connected to channel " + channelName + "!\n"));
	}
	// if (getChannel(channelName)->getTopic().empty())
	// 	reply(fd, RPL_NOTOPIC, channelName + " :No topic is set");
	// else
	// 	reply(fd, RPL_TOPIC, channelName + " :" + getChannel(channelName)->getTopic());
	// reply(fd, RPL_NAMREPLY, "= " + channelName + getClient(fd)->getNick());
	// reply(fd, RPL_ENDOFNAMES, channelName + " :End of user's list.");
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
	if (input.size() == 1)
		return ;
	if (getClient(fd)->isConnected() == true)
		return (reply(fd, ERR_ALREADYREGISTERED, ":Unauthorized command (already registered)"));
	if (strcmp(input[1].c_str(), Mdp))
	{
		std::cout << "Wrong password" << std::endl;
		reply(fd, ERR_PASSWDMISMATCH, ":Password incorrect");
		//messageFromServer(fd, "Wrong password!\n");
	}
	else
	{
		std::cout << "Good password" << std::endl;
		messageFromServer(fd, "Good password\n");
		getClient(fd)->setConnexion(true);
		getClient(fd)->setAuthorization(fd, true);
	}
}

void	Server::user(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return;
	std::vector<Client>::iterator client = getClient(fd);
	if (client->isConnected() == false)
	{
		messageFromServer(fd, "You have to enter the password first (/PASS <password>)\n");
		return;
	}
	if (client != clients.end())
	{
		if (client->getUser().empty() == false && client->isConnected())
		{
			messageFromServer(fd, "You already have a username: " + client->getUser() + '\n');
			return;
		}
		client->setUser(input[1]);
		std::string msg = ":yrio!~yrio@localhost USER :" + client->getUser() + "\r\n";
		std::cout << "User name set to " << input[1] << std::endl;
		// messageFromServer(fd, std::string("User name set to " + input[1] + '\n'));
	}
}

void	Server::nick(const int& fd, const std::vector<std::string>& input)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client == clients.end())
		return;
	if (client->isConnected() == false)
	{
		messageFromServer(fd, "You have to enter the password first (/PASS <password>)\n");
		return;
	}
	// if (client->getUser().empty())
	// {
	// 	messageFromServer(fd, "You have to enter your username first (/USER <username>)\n");
	// 	return;
	// }
	if (getClient(input[1]) != clients.end())
	{
		messageFromServer(fd, "This nickname is already used\n");
		return;
	}
	if (input.size() == 1)
		return;
	if (client != clients.end())
	{
		client->setNick(input[1]);
		std::string msg = ":yrio!~yrio@localhost NICK :" + client->getNick() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
		// messageFromServer(fd, "Nick name set to " + input[1] + "\n");
		std::cout << "Nick name set to " << input[1] << std::endl;
		client->setAuthorization(fd, true);
	}
}

void	Server::pong(const int fd, std::string token)
{
	const std::string message = "PONG " + token + "\r\n";
	send(fd, &message, message.size(), 0);
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
 	if (!getClient(fd)->isAdmin(*getChannel(channelName)))
	{
		messageFromServer(fd, "You do not have the right to kick someone\n");
		return;
	}
	for (size_t i = 2 ; i < input.size() ; i++)
	{
		if (getClient(input[i]) == clients.end() || !getClient(input[i])->isInChannel(channelName))
			messageFromServer(fd, "There is no user named " + input[i] + " in the channel + " + channelName + '\n');
		else
		{
			getClient(input[i])->deleteChannel(channelName);
			getChannel(channelName)->deleteClient(getClient(input[i])->getFd());
			getChannel(channelName)->deleteAdmin(getClient(input[i])->getFd());
			if (getChannel(channelName)->getClientCount() == 0)
				deleteChannel(channelName);
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
			reply(fd, RPL_INVITING, channelName + " " + input[i]);
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
			reply(fd, RPL_NOTOPIC, channelName + " :No topic is set");
		else
			reply(fd, RPL_TOPIC, channelName + " :" + getChannel(channelName)->getTopic());
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

void	Server::mode(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	std::string channelName = input[1];
	if (getChannel(channelName) == _channels.end())
	{
		messageFromServer(fd, "There is no channel named " + channelName + "\n");
		return;
	}
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