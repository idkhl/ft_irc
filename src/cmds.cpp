#include "../include/Server.hpp"
#include "../include/Client.hpp"

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
		return ;
	}
	if (input.size() == 1)
		return (reply(fd, ERR_NEEDMOREPARAMS, input[0] + " :Not enough parameters"));
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
			reply(fd, ERR_INVITEONLYCHAN, channelName + " :Cannot join channel (+i)");
			return;
		}
		if (getChannel(channelName)->getClientLimit() > 0 && getChannel(channelName)->getClientCount() == getChannel(channelName)->getClientLimit())
		{
			reply(fd, ERR_CHANNELISFULL, channelName + " :Cannot join channel (+l)");
			return;
		}
		if (getClient(fd)->getChannel().empty() == false)
			getChannel(getClient(fd)->getChannel())->deleteClient(fd);
		if (getChannel(channelName)->getPassword().empty() == false)
		{
			if (input.size() != 3)
			{
				reply(fd, ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
				return;
			}
			if (strcmp(input[2].c_str(), (getChannel(channelName)->getPassword()).c_str()) != 0)
			{
				std::cout << "Wrong password" << std::endl;
				reply(fd, ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
				return ;
			}
		}
		getClient(fd)->setChannel(channelName);
		if (std::find(getChannel(channelName)->getAdmins().begin(), getChannel(channelName)->getAdmins().end(), fd) != getChannel(channelName)->getAdmins().end())
			getClient(fd)->setAdmin(true);
		getChannel(channelName)->join(*getClient(fd));
		std::cout << "Connected to channel " << channelName << "!" << std::endl;
		messageFromServer(fd, std::string("Connected to channel " + channelName + "!\n"));
		reply(fd, RPL_TOPIC, channelName + " :" + getChannel(channelName)->getTopic());
		reply(fd, RPL_NAMREPLY, "= " + channelName + getClient(fd)->getNick());
	}
	// if (getChannel(channelName)->getTopic().empty())
	// 	reply(fd, RPL_NOTOPIC, channelName + " :No topic is set");
	// else
	// 	reply(fd, RPL_TOPIC, channelName + " :" + getChannel(channelName)->getTopic());
	// reply(fd, RPL_NAMREPLY, "= " + channelName + getClient(fd)->getNick());
	// reply(fd, RPL_ENDOFNAMES, channelName + " :End of user's list.");
}

void	Server::quit(const int& fd)
{
	ClearClients(fd);
}

void	Server::pass(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return (reply(fd, ERR_NEEDMOREPARAMS, input[0] + " :Not enough parameters"));
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
		messageFromServer(fd, "You are connected!\n");
		getClient(fd)->setConnexion(true);
		getClient(fd)->setAuthorization(fd, true);
	}
}

void	Server::user(const int& fd, const std::vector<std::string>& input)
{
	std::cout << "USER()" << std::endl;
	if (input.size() == 1)
		return (reply(fd, ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::vector<Client>::iterator client = getClient(fd);
	if (client != clients.end())
	{
		if (client->isAllowed())
			return (reply(fd, ERR_ALREADYREGISTERED, ":Unauthorized command (already registered)"));
		client->setUser(input[1]);
		// std::string msg = ":idakhlao!~idakhlao@localhost USER :" + client->getUser() + "\r\n";
		std::cout << "User name set to " << input[1] << std::endl;
		// messageFromServer(fd, std::string("User name set to " + input[1] + '\n'));
	}
}

bool isSpecial(char c)
{
    std::string specials = "[]\\`_^{|}";
    return specials.find(c) != std::string::npos;
}

bool isValidNickname(const std::string& nickname)
{
    if (nickname.empty() || nickname.size() > 9)
        return false;

    char first = nickname[0];
    if (!std::isalpha(first) && !isSpecial(first))
        return false;

    for (size_t i = 1; i < nickname.size(); ++i) {
        char c = nickname[i];
        if (!std::isalpha(c) && !std::isdigit(c) && !isSpecial(c) && c != '-') {
            return false;
        }
    }

    return true;
}

void	Server::nick(const int& fd, const std::vector<std::string>& input)
{
	if (input.size() == 1)
		return (reply(fd, ERR_NONICKNAMEGIVEN, ":No nickname given"));
	std::vector<Client>::iterator client = getClient(fd); 
	if (client != clients.end())
	{
		if (isValidNickname(input[1]))
		{
			std::string oldNick = client->getNick();
			std::cout << "OLDNICK: " << client->getNick() << std::endl;
			client->setNick(input[1]);
			std::string msg = ":" + oldNick + "!~idakhlao@localhost NICK " + client->getNick() + "\r\n";
			send(fd, msg.c_str(), msg.length(), 0);
			std::cout << "Nick name set to " << input[1] << std::endl;
		}
		else
			return (reply(fd, ERR_ERRONEUSNICKNAME, input[1] + " :Erroneous nickname"));

        // ERR_NICKNAMEINUSE              
        // ERR_UNAVAILRESOURCE            
	}
}
void	Server::pong(const int fd, std::string token)
{
	const std::string message = "PONG " + token + "\r\n";
	send(fd, &message, message.size(), 0);
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
			reply(fd, RPL_INVITING, getClient(fd)->getChannel() + " " + usersToInvite[i]);
		}
	}
}

void	Server::topic(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->getChannel().empty())
		return;
	if (input.size() == 1)
	{
		if (getChannel(getClient(fd)->getChannel())->getTopic().empty())
			reply(fd, RPL_NOTOPIC, getClient(fd)->getChannel() + " :No topic is set");
		else
			reply(fd, RPL_TOPIC, getClient(fd)->getChannel() + " :" + getChannel(getClient(fd)->getChannel())->getTopic());
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
	if (!getClient(fd)->isAllowed())
	{
		messageFromServer(fd, "You have to enter the password, username and nickname first\n");
		return;
	}
	if (input.size() < 3)
	{
		reply(fd, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
		return;
	}

	std::string target = input[1];
	std::string message;
	for (size_t i = 2; i < input.size(); ++i)
	{
		if (!message.empty())
			message += " ";
		message += input[i];
	}

	std::vector<Channel>::iterator chanIt = getChannel(target);
	if (chanIt != _channels.end())
	{
		std::vector<Client*> members = chanIt->getClients();
		for (size_t i = 0; i < members.size(); ++i)
		{
			if (members[i]->getFd() != fd)
			{
				std::string channelMsg = ":" + getClient(fd)->getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
				send(members[i]->getFd(), channelMsg.c_str(), channelMsg.size(), 0);
			}
		}
		return;
	}

	std::vector<Client>::iterator userIt = getClient(target);
	if (userIt != clients.end())
	{
		std::string privMsg = ":" + getClient(fd)->getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
		send(userIt->getFd(), privMsg.c_str(), privMsg.size(), 0);
		return;
	}

	reply(fd, ERR_NOSUCHNICK, target + " :No such nick/channel");
}
