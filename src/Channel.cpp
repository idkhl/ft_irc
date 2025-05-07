#include "../include/Channel.hpp"

Channel::Channel(Client& client, const std::string& name) : _name(name)
{
	client.setAdmin(true);
	_adminFds.push_back(client.getFd());
	_clients.push_back(client);
	pollfd clientFd = {
		.fd = client.getFd(),
		.events = POLLIN,
		.revents = 0,
	};
	_pollfds.push_back(clientFd);
}

void	Channel::sendMessage(const std::string& message) const
{
	for (size_t i = 0 ; i < _clients.size() ; i++)
		send(_clients[i].getFd(), message.c_str(), strlen(message.c_str()), 0);
}

void	Channel::join(Client& client)
{
	_clients.push_back(client);
	pollfd clientFd = {
		.fd = client.getFd(),
		.events = POLLIN,
		.revents = 0,
	};
	_pollfds.push_back(clientFd);
}

void	Channel::deleteClient(const int& fd)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client != _clients.end())
	{
		client->getChannel().clear();
		_clients.erase(client);
		std::vector<pollfd>::const_iterator it;
		size_t i;
		for (i = 0 ; i < _pollfds.size() ; i++)
		{
			if (_pollfds[i].fd == fd)
				break;
		}
		if (i < _pollfds.size())
			_pollfds.erase(_pollfds.begin() + i);
	}
}