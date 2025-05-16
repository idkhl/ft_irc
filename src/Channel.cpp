#include "../include/Channel.hpp"

Channel::Channel(Client& client, const std::string& name) : _name(name)
{
	client.setAdmin(true);
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

void	Channel::join(Client& client)
{
	if (std::find(_adminFds.begin(), _adminFds.end(), client.getFd()) != _adminFds.end())
		client.setAdmin(true);
	else
		client.setAdmin(false);
	_clients.push_back(&client);
}

void	Channel::deleteClient(const int& fd)
{
	Client *client = getClient(fd);
	if (client != NULL)
	{
		client->getChannel().clear();
		client->setAdmin(false);
		size_t i;
		for (i = 0 ; i < _clients.size() ; i++)
		{
			if (_clients[i]->getFd() == fd)
				break;
		}
		_clients.erase(_clients.begin() + i);
	}
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