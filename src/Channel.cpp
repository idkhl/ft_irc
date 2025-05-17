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
	for (size_t i = 0 ; i < _clients.size() ; i++)
		send(_clients[i]->getFd(), message.c_str(), strlen(message.c_str()), 0);
}

void	Channel::deleteClient(const int& fd)
{
	std::vector<Client *>::iterator client = std::find(_clients.begin(),_clients.end(), getClient(fd));
	if (client != _clients.end())
		_clients.erase(client);
}

void	Channel::deleteAdmin(const int& fd)
{
	std::vector<int>::iterator client = std::find(_adminFds.begin(),_adminFds.end(), getClient(fd)->getFd());
	if (client != _adminFds.end())
		_adminFds.erase(client);
}

Client	*Channel::getClient(const int& fd)
{
	for (size_t i = 0 ; i < _clients.size() ; i++)
	{
		if (_clients[i]->getFd() == fd)
			return _clients[i];
	}
	return NULL;
}

Client	*Channel::getClient(const std::string& userName)
{
	for (size_t i = 0 ; i < _clients.size() ; i++)
	{
		if (_clients[i]->getUser() == userName)
			return _clients[i];
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
	if (getClient(client.getFd()) == NULL)
		_clients.push_back(&client);
}