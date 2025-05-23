#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Client.hpp"

void	Server::join(const int& fd, const std::vector<std::string>& input)
{
	if (getClient(fd)->isConnected() == false)
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
		return;
	}
	if (input.size() == 1)
		return (reply(fd, ERR_NEEDMOREPARAMS, input[0] + " :Not enough parameters"));
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
			return;
		}
		if (getChannel(channelName)->getClientLimit() > 0 && getChannel(channelName)->getClientCount() == getChannel(channelName)->getClientLimit())
		{
			reply(fd, ERR_CHANNELISFULL, channelName + " :Cannot join channel (+l)");
			return;
		}
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
				return;
			}
		}
		getChannel(channelName)->addClient(*getClient(fd));
		std::cout << "Connected to channel " << channelName << "!" << std::endl;
		messageFromServer(fd, std::string("Connected to channel " + channelName + "!\n"));
	}
	if (!getChannel(channelName)->getTopic().empty())
		reply(fd, RPL_TOPIC, channelName + " :" + getChannel(channelName)->getTopic());
	reply(fd, RPL_NAMREPLY, "= " + channelName + getClient(fd)->getNick());
}

void	Server::quit(const int& fd)
{
	ClearClients(fd);
}

void	Server::pass(const int& fd, const std::vector<std::string>& input, int index)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client == clients.end())
		return;
	if (input.size() == 1)
		return (reply(fd, ERR_NEEDMOREPARAMS, input[0] + " :Not enough parameters"));
	if (getClient(fd)->isConnected() == true)
		return (reply(fd, ERR_ALREADYREGISTERED, ":Unauthorized command (already registered)"));
	size_t pos = input[index].find('\r', 0);
	std::string sub_mdp = input[index].substr(0, pos);
	if (strcmp(sub_mdp.c_str(), Mdp))
	{
		std::cout << "Wrong password" << std::endl;
		reply(fd, ERR_PASSWDMISMATCH, " :Password incorrect");
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
	std::cout << "USER()" << std::endl;
	if (input.size() == 1)
		return (reply(fd, ERR_NEEDMOREPARAMS, " :Not enough parameters"));
	std::vector<Client>::iterator client = getClient(fd);
	if (client->isConnected() == false)
	{
		messageFromServer(fd, "You have to enter the password first (/PASS <password>)\n");
		return;
	}
	if (client != clients.end())
	{
		if (client->isAllowed())
			return (reply(fd, ERR_ALREADYREGISTERED, ":Unauthorized command (already registered)"));
		client->setUser(input[index]);
		std::string msg = ":yrio!~yrio@localhost USER :" + client->getUser() + "\r\n";
		std::cout << "User name set to " << input[index] << std::endl;
		// messageFromServer(fd, std::string("User name set to " + input[1] + '\n'));
		if (getClient(fd)->isConnected() == true && !getClient(fd)->getNick().empty() && !getClient(fd)->getUser().empty())
			getClient(fd)->setAuthorization(fd, true);
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
			std::cout << "Test 3" << std::endl;
            return false;
        }
    }
    return true;
}

void	Server::nick(const int& fd, const std::vector<std::string>& input, int index)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client == clients.end())
		return (reply(fd, ERR_ERRONEUSNICKNAME, input[1] + " :Erroneous nickname"));
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
		return (reply(fd, ERR_NONICKNAMEGIVEN, ":No nickname given"));
	size_t pos = input[index].find('\r', 0);
	std::string sub_nick = input[index].substr(0, pos);
	if (isValidNickname(sub_nick))
	{
		std::string oldNick = client->getNick();
		// std::cout << "OLDNICK: " << client->getNick() << std::endl;
		client->setNick(sub_nick);
		std::string msg = ":" + oldNick + "!~yrio@localhost NICK " + client->getNick() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
		std::cout << "Nick name set to " << sub_nick << std::endl;
	}
	// ERR_NICKNAMEINUSE              
	// ERR_UNAVAILRESOURCE
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
	if (!getClient(fd)->isAllowed())
	{
		messageFromServer(fd, "You have to be connected first:\n/USER\n/NICK\n/PASS\n");
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