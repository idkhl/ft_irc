#include "../include/Channel.hpp"
#include "../include/Client.hpp"

Channel::Channel(Client& client, const std::string& name) : _name(name)
{
	_adminFds.push_back(client.getFd());
	_clients.push_back(&client);
	_topicRestriction = false;
	_inviteMode = false;
	_clientLimit = -1;
}

Channel::Channel(Client& client, const std::string& name, const std::string& topic) :_name(name), _topic(topic)
{
	_adminFds.push_back(client.getFd());
	_clients.push_back(&client);
	_topicRestriction = false;
	_inviteMode = false;
	_clientLimit = -1;
}

void	Channel::sendMessage(const std::string& message) const
{
	for (std::list<Client *>::const_iterator client = _clients.begin() ; client != _clients.end() ; ++client)
		send((*client)->getFd(), message.c_str(), strlen(message.c_str()), 0);
}

void	Channel::deleteClient(const int& fd)
{
	for (std::list<Client *>::iterator client = _clients.begin() ; client != _clients.end() ; ++client)
	{
		if ((*client)->getFd() == fd)
		{
			_clients.erase(client);
			return;
		}
	}
}

Client	*Channel::getClient(const int& fd)
{
	for (std::list<Client *>::const_iterator client = _clients.begin() ; client != _clients.end() ; ++client)
	{
		if ((*client)->getFd() == fd)
			return *client;
	}
	return NULL;
}

Client	*Channel::getClient(const std::string& userName)
{
	for (std::list<Client *>::const_iterator client = _clients.begin() ; client != _clients.end() ; ++client)
	{
		if ((*client)->getUser() == userName)
			return *client;
	}
	return NULL;
}

Client	*Channel::getAdmin(const int& fd)
{
	std::vector<int>::const_iterator clientFd = std::find(_adminFds.begin(), _adminFds.end(), fd);
	return clientFd != _adminFds.end() ? getClient(*clientFd) : NULL;
}

Client	*Channel::getAdmin(const std::string& userName)
{
	Client *client = getClient(userName);
	if (client == NULL)
		return NULL;
	return getAdmin(client->getFd());
}

void	Channel::addClient(Client& client)
{
	if (getClient(client.getFd()) == NULL && ((int)_clientLimit != -1 || (int)getClientCount() < _clientLimit))
		_clients.push_back(&client);
}

void	Channel::deleteAdmin(const int& fd)
{
	std::vector<int>::iterator admin = std::find(_adminFds.begin(), _adminFds.end(), fd);
	if (admin != _adminFds.end())
		_adminFds.erase(admin);
}