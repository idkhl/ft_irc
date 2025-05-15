#include "../include/Channel.hpp"
#include "../include/Client.hpp"

Channel::Channel(Client& client, const std::string& name) : _name(name)
{
	_adminFds.push_back(client.getFd());
	_clients.push_back(&client);
	_topicRestriction = false;
	_inviteMode = false;
}

Channel::Channel(Client& client, const std::string& name, const std::string& topic) :_name(name), _topic(topic)
{
	_adminFds.push_back(client.getFd());
	_clients.push_back(&client);
	_topicRestriction = false;
	_inviteMode = false;
}

void	Channel::sendMessage(const std::string& message) const
{
	for (size_t i = 0 ; i < _clients.size() ; i++)
		send(_clients[i]->getFd(), message.c_str(), strlen(message.c_str()), 0);
}

void	Channel::deleteClient(const int& fd)
{
	Client *client = getClient(fd);
	if (client != NULL)
	{
		size_t i;
		for (i = 0 ; i < _clients.size() ; i++)
		{
			if (_clients[i]->getFd() == fd)
				break;
		}
		_clients.erase(_clients.begin() + i);
	}
}

void	Channel::deleteAdmin(const int& fd)
{
	if (getClient(fd) == NULL)
		return;
	size_t i;
	for (i = 0 ; i < _adminFds.size() ; i++)
	{
		if (_adminFds[i] == fd)
			break;
	}
	_adminFds.erase(_adminFds.begin() + i);
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