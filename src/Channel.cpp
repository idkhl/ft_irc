#include "../include/Channel.hpp"

Channel::Channel(Client& client, const std::string& name) : _name(name)
{
	std::cout << &client << std::endl;
	client.setAdmin(true);
	_adminFds.push_back(client.getFd());
	_clients.push_back(client);
}

void	Channel::sendMessage(const std::string& message) const
{
	for (size_t i = 0 ; i < _clients.size() ; i++)
		send(_clients[i].getFd(), message.c_str(), strlen(message.c_str()), 0);
}

void	Channel::join(Client& client)
{
	_clients.push_back(client);
}

void	Channel::deleteClient(const int& fd)
{
	std::vector<Client>::iterator client = getClient(fd);
	if (client != _clients.end())
	{
		client->getChannel().clear();
		_clients.erase(client);
	}
}