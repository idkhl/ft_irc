#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Client.hpp"

void	Server::join(const int& fd, const std::vector<std::string>& input)
{
	if (!getClient(fd)->isConnected())
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 2)
		return reply(getClient(fd), ERR_NEEDMOREPARAMS, input[0] + " :Not enough parameters");

	std::vector<std::string> channels;
	std::vector<std::string> keys;
	std::string::size_type start = 0, end;
	std::string chans = input[1];
	while ((end = chans.find(',', start)) != std::string::npos)
	{
		channels.push_back(chans.substr(start, end - start));
		start = end + 1;
	}
	channels.push_back(chans.substr(start));

	if (input.size() > 2)
	{
		start = 0;
		std::string ks = input[2];
		while ((end = ks.find(',', start)) != std::string::npos)
		{
			keys.push_back(ks.substr(start, end - start));
			start = end + 1;
		}
		keys.push_back(ks.substr(start));
	}

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string key = (i < keys.size()) ? keys[i] : "";
		joinChannel(fd, input, channels[i], key);
	}
}

void	Server::joinChannel(const int& fd, const std::vector<std::string>& /*input*/, std::string chan, std::string key)
{
	std::string channelName = (chan[0] == '#') ? chan : ('#' + chan);

	if (getChannel(channelName) == _channels.end())
	{
		getClient(fd)->addChannel(channelName);
		Channel channel(*getClient(fd), channelName);
		_channels.push_back(channel);
		getChannel(channelName)->addClient(*getClient(fd));
		messageFromServer(fd, std::string("Channel " + channelName + " created!\n"));
	}
	else
	{
		if (getChannel(channelName)->isInviteOnly() && !getClient(fd)->isInvitedIn(channelName))
			return reply(getClient(fd), ERR_INVITEONLYCHAN, getClient(fd)->getNick() + " " + channelName + " :Cannot join channel (+i)");
		if (getChannel(channelName)->getClientLimit() > 0 && (int)getChannel(channelName)->getClientCount() == getChannel(channelName)->getClientLimit())
			return reply(getClient(fd), ERR_CHANNELISFULL, getClient(fd)->getNick() + " " + channelName + " :Cannot join channel (+l)");
		if (!getChannel(channelName)->getPassword().empty())
		{
			if (key.empty() || strcmp(key.c_str(), getChannel(channelName)->getPassword().c_str()) != 0)
			{
				reply(getClient(fd), ERR_BADCHANNELKEY, getClient(fd)->getNick() + " " + channelName + " :Cannot join channel (+k)");
				return;
			}
		}
		getChannel(channelName)->addClient(*getClient(fd));
		getClient(fd)->addChannel(channelName);
		messageFromServer(fd, std::string("Connected to channel " + channelName + "!\n"));
	}
	std::string message = ":" + getClient(fd)->getNick() + "!" + getClient(fd)->getUser() + "@localhost JOIN :" + channelName + "\r\n";
	std::list<Client *> clients = getChannel(channelName)->getClients();
	for (std::list<Client *>::const_iterator client = clients.begin(); client != clients.end(); ++client)
		send((*client)->getFd(), message.c_str(), message.size(), 0);

	std::string userList = ":";
	Channel& channelToJoin = *getChannel(channelName);
	for (std::list<Client *>::const_iterator it = channelToJoin.getClients().begin(); it != channelToJoin.getClients().end(); ++it) 
	{
		if (std::find(channelToJoin.getAdmins().begin(), channelToJoin.getAdmins().end(), (*it)->getFd()) != channelToJoin.getAdmins().end())
			userList += "@" + (*it)->getNick() + " ";
		else
			userList += (*it)->getNick() + " ";
	}
	if (userList.size() > 1)
		userList = userList.substr(0, userList.size() - 1);

	std::string nameReply = ":localhost 353 " + getClient(fd)->getNick() + " = " + channelName + " " + userList + "\r\n";
	send(fd, nameReply.c_str(), nameReply.size(), 0);
	std::string endOfNames = ":localhost 366 " + getClient(fd)->getNick() + " " + channelName + " :End of list.\r\n";
	send(fd, endOfNames.c_str(), endOfNames.size(), 0);

	if (!channelToJoin.getTopic().empty()) 
	{
		std::string topicMessage = ":localhost 332 " + getClient(fd)->getNick() + " " + channelName + " :" + channelToJoin.getTopic() + "\r\n";
		send(fd, topicMessage.c_str(), topicMessage.size(), 0);
	}
}

void	Server::quit(const int& fd)
{
	ClearClients(fd);
}

void	Server::pass(const int& fd, const std::vector<std::string>& input, int index)
{
	std::list<Client>::iterator client = getClient(fd);
	if (client == clients.end())
		return;
	if (input.size() == 1)
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, input[0] + " :Not enough parameters"));
	if (getClient(fd)->isConnected() == true)
		return (reply(getClient(fd), ERR_ALREADYREGISTERED, ":Unauthorized command (already registered)"));
	size_t pos = input[index].find('\r', 0);
	std::string sub_mdp = input[index].substr(0, pos);
	if (strcmp(sub_mdp.c_str(), Mdp))
	{
		std::cout << "Wrong password" << std::endl;
		reply(getClient(fd), ERR_PASSWDMISMATCH, " :Password incorrect");
		//messageFromServer(fd, "Wrong password!\n");
	}
	else
	{
		std::cout << "Good password" << std::endl;
		messageFromServer(fd, "Good password\n");
		getClient(fd)->setConnexion(true);
		if (!getClient(fd)->getNick().empty() && !getClient(fd)->getUser().empty())
			getClient(fd)->setAuthorization(fd, true);
	}
}

void	Server::user(const int& fd, const std::vector<std::string>& input, int index)
{
	if (input.size() == 1)
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::list<Client>::iterator client = getClient(fd);
	if (client->isConnected() == false)
	{
		messageFromServer(fd, "You have to enter the password first (/PASS <password>)\n");
		return;
	}
	if (client != clients.end())
	{
		if (client->isAllowed())
			return (reply(getClient(fd), ERR_ALREADYREGISTERED, ":Unauthorized command (already registered)"));
		client->setUser(input[index]);
		std::cout << "User name set to " << input[index] << std::endl;
		if (getClient(fd)->isConnected() == true && !getClient(fd)->getNick().empty() && !getClient(fd)->getUser().empty())
			getClient(fd)->setAuthorization(fd, true);
		// sendToIrssi(client, ":" + client->getNick() + " USER :" + client->getUser());
	}
}

bool	isSpecial(char c)
{
    std::string specials = "[]\\`_^{|}";
    return specials.find(c) != std::string::npos;
}

bool	isValidNickname(const std::string& nickname)
{
    if (nickname.empty() || nickname.size() > 9)
        return false;
    char first = nickname[0];
    if (!std::isalpha(first) && !isSpecial(first))
	{
        return false;
	}
	for (size_t i = 1; i < nickname.size(); ++i)
	{
		char c = nickname[i];
		if (!std::isalnum(c) && !isSpecial(c))
		{
			return false;
		}
	}
    return true;
}

void	Server::nick(const int& fd, const std::vector<std::string>& input, int index)
{
	if (input.size() == 1)
		return (reply(getClient(fd), ERR_NONICKNAMEGIVEN, ":No nickname given"));
	std::list<Client>::iterator client = getClient(fd);
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
		return(reply(getClient(fd), ERR_NICKNAMEINUSE, input[1] + " :Nickname is already in use"));
	size_t pos = input[index].find('\r', 0);
	std::string sub_nick = input[index].substr(0, pos);
	if (isValidNickname(sub_nick))
	{
		std::string oldNick = client->getNick();
		// std::cout << "OLDNICK: " << client->getNick() << std::endl;
		client->setNick(sub_nick);
		std::string msg = ":" + oldNick + "!" + client->getUser() + "@localhost NICK " + client->getNick() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
		std::cout << "Nick name set to " << sub_nick << std::endl;
	}
	else
		return (reply(getClient(fd), ERR_ERRONEUSNICKNAME, input[1] + " :Erroneous nickname"));
	// ERR_UNAVAILRESOURCE
}

void	Server::pong(const int& fd, std::string token)
{
	std::string message = "PONG " + token + "\r\n";
	std::cout << message << std::endl;
	send(fd, message.c_str(), message.size(), 0);
}

void	Server::kick(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 3)
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::string channelName = input[1];
	if (getChannel(channelName) == _channels.end())
		return (reply(getClient(fd), ERR_NOSUCHCHANNEL, channelName + " :No such channel"));
	if (!getClient(fd)->isInChannel(channelName))
		return (reply(getClient(fd), ERR_NOTONCHANNEL, channelName + " :You're not on that channel"));
	if (!getClient(fd)->isAdmin(*getChannel(channelName)))
		return (reply(getClient(fd), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"));

	std::string target = input[2];
	if (getClient(target) == clients.end() || !getClient(target)->isInChannel(channelName))
	{
		for (size_t i = 0 ; i < getChannel(channelName)->getClients().size() ; i++)
			std::cout << (*getChannel(channelName)->getClients().begin())->getNick() << std::endl;
		reply(getClient(fd), ERR_USERNOTINCHANNEL, input[2] + " " + channelName + " :They aren't on that channel");
	}
	else
	{
		getClient(target)->deleteChannel(channelName);
		getChannel(channelName)->deleteClient(getClient(target)->getFd());
		getChannel(channelName)->deleteAdmin(getClient(target)->getFd());
		if (getChannel(channelName)->getClientCount() == 0)
			deleteChannel(channelName);
		std::string message = ":" + getClient(fd)->getNick() + " KICK " + channelName + " " + target;
		if (input.size() > 4)
		{
			std::string reason;
			for (size_t i = 3 ; i < input.size() ; i++)
				reason += i == input.size() - 1 ? input[i] : input[i] + ' ';
			message += " :" + reason;
		}
		message += "\r\n";
		std::cout << message << std::endl;
		for (size_t i = 0 ; i < getChannel(channelName)->getClientCount() ; i++)
			send((*getChannel(channelName)->getClients().begin())->getFd(), message.c_str(), message.size(), 0);
		send(getClient(target)->getFd(), message.c_str(), message.size(), 0);
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
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::string channelName = input[2];
	std::string target = input[1];
	if (getClient(fd)->isAdmin(*getChannel(channelName)) == false)
		return (reply(getClient(fd), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"));
	if (getChannel(channelName) == _channels.end())
		return (reply(getClient(fd), ERR_NOSUCHCHANNEL, channelName + " :No such channel"));
	if (getClient(target) == clients.end())
		return (reply(getClient(fd), ERR_NOSUCHNICK, target + " :No such nick/channel"));
	if (getClient(target)->isInChannel(channelName))
		return reply(getClient(fd), ERR_USERONCHANNEL, target + " " + channelName + " :is already on channel");
	if (!getClient(fd)->isInChannel(channelName))
		return reply(getClient(fd), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");

	getClient(target)->addInvitation(channelName);
	reply(getClient(fd), RPL_INVITING, getClient(fd)->getNick() + " " + target + " " + channelName);
	std::string message = " INVITE " + target + " :" + channelName;
	sendToIrssi(getClient(target), message);
}

void	Server::topic(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 2)
		return (reply(getClient(fd), ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::string channelName = input[1];
	if (getChannel(channelName) == _channels.end())
		return (reply(getClient(fd), ERR_NOSUCHCHANNEL, channelName + " :No such channel"));
	if (input.size() == 2)
	{
		if (getChannel(channelName)->getTopic().empty())
			reply(getClient(fd), RPL_NOTOPIC, channelName + " :No topic is set");
		else
			reply(getClient(fd), RPL_TOPIC, channelName + " :" + getChannel(channelName)->getTopic());
		return;
	}
	if (!getClient(fd)->isAdmin(*getChannel(channelName)) && getChannel(channelName)->isTopicRestriction())
		return (reply(getClient(fd), ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"));
	if (!getClient(fd)->isInChannel(channelName))
		return reply(getClient(fd), ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
	std::string topic;
	for (size_t i = 2 ; i < input.size() ; i++)
	{
		topic += input[i];
		if (i != input.size() - 1)
			topic += ' ';
	}
	getChannel(channelName)->setTopic(topic);
	std::string message = ":" + getClient(fd)->getNick() + " TOPIC " + channelName + " " + topic + "\r\n";
	for (size_t i = 0 ; i < getChannel(channelName)->getClientCount() ; i++)
		send((*getChannel(channelName)->getClients().begin())->getFd(), message.c_str(), message.size(), 0);
}

void	Server::msg(const int& fd, const std::vector<std::string>& input)
{
	if (!getClient(fd)->isAllowed())
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() < 3)
	{
		reply(getClient(fd), ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
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
	std::list<Channel>::iterator chanIt = getChannel(target);
	if (chanIt != _channels.end())
	{
		std::list<Client *> members = chanIt->getClients();
		for (std::list<Client *>::const_iterator member = members.begin() ; member != members.end() ; ++member)
		{
			if ((*member)->getFd() != fd)
			{
				std::cout << "client : " << (*member)->getNick() << std::endl;
				std::string channelMsg = ":" + getClient(fd)->getNick() + " PRIVMSG " + target + " " + message + "\r\n";
				send((*member)->getFd(), channelMsg.c_str(), channelMsg.size(), 0);
			}
		}
		return;
	}
	std::list<Client>::iterator userIt = getClient(target);
	if (userIt != clients.end())
	{
		std::string privMsg = ":" + getClient(fd)->getNick() + " PRIVMSG " + target + " " + message + "\r\n";
		send(userIt->getFd(), privMsg.c_str(), privMsg.size(), 0);
		return;
	}
	reply(getClient(fd), ERR_NOSUCHNICK, target + " :No such nick/channel");
}