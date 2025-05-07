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
	messageFromServer(fd, std::string("You left the channel " + getClient(fd)->getChannel() + "\n"));
	getChannel(getClient(fd)->getChannel())->deleteClient(fd);
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
	if (getChannel(input[1]) == _channels.end())
	{
		if (getClient(fd)->getChannel().empty() == false)
			getChannel(getClient(fd)->getChannel())->deleteClient(fd);
		getClient(fd)->setChannel(input[1]);
		_channels.push_back(Channel(*getClient(fd), input[1]));
		std::cout << "Channel " << input[1] << " created!" << std::endl;
		messageFromServer(fd, std::string("Channel " + input[1] + " created!\n"));
	}
	else
	{
		if (getClient(fd)->getChannel().empty() == false)
			getChannel(getClient(fd)->getChannel())->deleteClient(fd);
		getClient(fd)->setChannel(input[1]);
		if (std::find(getChannel(input[1])->getAdmins().begin(), getChannel(input[1])->getAdmins().end(), fd) != getChannel(input[1])->getAdmins().end())
			getClient(fd)->setAdmin(true);
		getChannel(input[1])->join(*getClient(fd));
		std::cout << "Connected to channel " << input[1] << "!" << std::endl;
		messageFromServer(fd, std::string("Connected to channel " + input[1] + "!\n"));
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

void	Server::kick(const int& fd, const std::string& user)
{
	if (getClient(fd)->isAdmin() == false)
	{
		messageFromServer(fd, "You do not have the right to kick someone\n");
		return;
	}
	std::vector<Client>::iterator userToKick = getClient
}