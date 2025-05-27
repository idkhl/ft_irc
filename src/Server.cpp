#include "../include/Server.hpp"
#include <string.h>

bool Server::Signal = false;

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}

void Server::CloseFds()
{
	for (std::list<Client>::iterator it = clients.begin() ; it != clients.end() ; ++it)
	{
		std::cout << RED << "Client <" << it->getFd() << "> Disconnected" << WHITE << std::endl;
		close(it->getFd());
	}
	if (this->ServSocket != -1)
	{
		std::cout << RED << "Server <" << this->ServSocket << "> Disconnected" << WHITE << std::endl;
		close(this->ServSocket);
	}
}

void Server::SerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET;
	add.sin_port = htons(this->Port);
	add.sin_addr.s_addr = INADDR_ANY;
	int en = 1;

	this->ServSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(this->ServSocket == -1)
		throw(std::runtime_error("faild to create socket"));

	if(setsockopt(this->ServSocket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(this->ServSocket, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(this->ServSocket, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(this->ServSocket, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = ServSocket;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	this->fds.push_back(NewPoll);
}

void Server::AcceptIncomingClient()
{
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(this->ServSocket, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl; 
		return;
	}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "fcntl() failed" << std::endl; 
		return;
	}
	NewPoll.fd = incofd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;

	Client client(incofd, cliadd.sin_addr);
	client.setInterface(NC);
	this->clients.push_back(client);
	this->fds.push_back(NewPoll);
	std::cout << GREEN << "Client <" << incofd << "> Connected" << WHITE << std::endl;
	messageFromServer(incofd, "WELCOME IN FT_IRC!\nEnter your USERNAME, NICKNAME and PASSWORD\n");
}

void	Server::check_connexion(const int& fd, std::vector<std::string> input)
{
	if (input[0] == "CAP" && input.size() < 3)
		return ;
	else if (input[0] == "CAP" && input.size() > 3 && input[2] == "NICK")
		std::cout << "PASSWORD AND NICKNAME REQUIRED" << std::endl;
	else if (input[0] == "CAP" && input.size() == 4 && input[2] == "PASS")
		pass(fd, input, 3);
	else if (input[0] == "CAP" && input.size() == 6 && input[2] == "PASS")
	{
		pass(fd, input, 3);
		nick(fd, input, 5);
	}
	else if (input[0] == "CAP" && input.size() == 12)
	{
		pass(fd, input, 3);
		nick(fd, input, 5);
		user(fd, input, 7);
	}
	else if (input[0] == "PASS" && input.size() == 4)
	{
		pass(fd, input, 1);
		nick(fd, input, 3);
	}
	else if (input[0] == "PASS" && input.size() == 10)
	{
		pass(fd, input, 1);
		nick(fd, input, 3);
		user(fd, input, 5);
	}
	else if (input[0] == "NICK" && input.size() == 8)
	{
		nick(fd, input, 1);
		user(fd, input, 3);
	}
	else if (input[0] == "PASS" && input.size() == 2)
		pass(fd, input, 1);
	else if (input[0] == "NICK" && input.size() == 2 && getClient(fd)->isConnected() == true)
		nick(fd, input, 1);
	else if ((input[0] == "USER" || input[0] == "USERHOST") && !getClient(fd)->getNick().empty() && getClient(fd)->isConnected() == true)
		user(fd, input, 1);
}

static std::vector<std::string>	splitInput(std::string str)
{
	if (str.size() >= 2 && str.substr(str.size() - 2) == "\r\n")
		str.erase(str.size() - 2);

	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == '\n')
			str[i] = ' ';
	}
	std::vector<std::string> result;
	size_t start = 0;
	while (start < str.size())
	{
		while (start < str.size() && str[start] == ' ')
			start++;
		if (start >= str.size())
			break;
		size_t end = start;
		while (end < str.size() && str[end] != ' ')
			end++;
		result.push_back(str.substr(start, end - start));
		start = end;
	}
	return result;
}

void	Server::handleCmd(const int& fd, char *buff)
{
	std::vector<std::string> input = splitInput(buff);
	if (input.empty())
		return;
	std::string cmd = input[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), toupper);
	// std::cout << "[" << cmd << "]" << std::endl;
	if (cmd == "CAP" || cmd == "PASS" || cmd == "NICK" || cmd == "USER")
		return (check_connexion(fd, input));
	else if (cmd == "QUIT")
		quit(fd);
	else if (cmd == "JOIN")
		join(fd, input);
	else if (cmd == "PRIVMSG")
		msg(fd, input);
	if (getClient(fd)->getChannels().size() == 0)
		return;
	else if (cmd == "KICK")
		kick(fd, input);
	else if (cmd == "INVITE")
		invite(fd, input);
	else if (cmd == "MODE")
		mode(fd, input);
	else if (cmd == "TOPIC")
		topic(fd, input);
	else if (cmd == "PING")
		pong(fd, input.size() > 1 ? input[1] : "localhost");
	// else
	// 	broadcastToChannel(input);
}

// void	Server::broadcastToChannel(const std::vector<std::string>& input)
// {
// 	std::string channelName = input[1];
// 	std::list<Channel>::iterator channel = getChannel(channelName);
// 	std::string message;
// 	for (size_t i = 2 ; i < input.size() ; i++)
// 		message += i == input.size() - 1 ? input[i] : input[i] + ' ';
// 	if (channel != _channels.end())
// 		channel->sendMessage(message);
// }

void	 Server::ReceiveDataClient(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);

	std::cout << "buffer = " << buff << std::endl;

	if (bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHITE << std::endl;
		ClearClients(fd);
		close(fd);
	}
	else
	{
		buff[bytes] = '\0';
		//here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
		handleCmd(fd, buff);
	}
}

static void	parseArgs(const int& port, const char *mdp)
{
	if (port <= 1024 || port > 65535)
		throw std::runtime_error("port invalid");
	if (!(*mdp))
		throw std::runtime_error("the password cannot be empty");

}

void Server::ServerInit(int port, char *mdp)
{
	parseArgs(port, mdp);
	this->Port = port;
	this->Mdp = mdp;
	SerSocket();

	std::cout << GREEN << "Server <" << this->ServSocket << "> Connected" << WHITE << std::endl;
	std::cout << "Waiting to accept a connection..." << std::endl;

	while (Server::Signal == false)
	{
		if((poll(&this->fds[0], this->fds.size(), -1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (this->fds[i].revents & POLLIN)
			{
				if (this->fds[i].fd == this->ServSocket)
					AcceptIncomingClient();
				else
					ReceiveDataClient(this->fds[i].fd); 
			}
		}
	}
	CloseFds();
}

void Server::ClearClients(int fd)
{
	std::cout << RED << "Client <" << fd << "> Disconnected" << WHITE << std::endl;
	std::list<Client>::iterator client = getClient(fd);
	for (size_t i = 0 ; i < client->getChannels().size() ; i++)
	{
		std::list<Channel>::iterator channel = getChannel(client->getChannels()[i]);
		channel->deleteAdmin(fd);
		channel->deleteClient(fd);
		if (channel->getClientCount() == 0)
		{
			deleteChannel(channel);
			break;
		}
		if (channel->getNbrAdmins() == 0)
		{
			channel->addAdmin((*channel->getClients().begin())->getFd());
			messageFromServer((*channel->getClients().begin())->getFd(), "You are now an operator of the channel " + channel->getName() + '\n');	
		}
	}
	for (size_t i = 0; i < this->fds.size(); i++)
	{
		if (this->fds[i].fd == fd)
		{
			this->fds.erase(this->fds.begin() + i);
			break;
		}
	}
	clients.erase(getClient(fd));
}

void	reply(std::list<Client>::iterator client, std::string code, std::string msg)
{
	std::string response = ":localhost " + code;
	response += ' ' + msg + "\r\n";
	send(client->getFd(), response.c_str(), response.length(), 0);
}

void	sendToIrssi(std::list<Client>::iterator client, std::string message)
{
	std::string msg = ":localhost " + message + "\r\n";
	// std::cout << "SNDTOIRSSI [" << msg << "]" << std::endl;
	send(client->getFd(), msg.c_str(), msg.length(), 0);
}
void	Server::deleteChannel(const std::string& channelName)
{
	std::list<Channel>::iterator channel = getChannel(channelName);
	if (channel != _channels.end())
		_channels.erase(channel);
}