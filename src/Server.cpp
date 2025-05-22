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
	for(size_t i = 0; i < clients.size(); i++)
	{
		std::cout << RED << "Client <" << this->clients[i].getFd() << "> Disconnected" << WHITE << std::endl;
		close(this->clients[i].getFd());
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
}

static std::vector<std::string> splitInput(std::string str)
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
	// std::cout << "LAAAAAA  " + cmd << std::endl;
	if (cmd == "CAP")
		return;
	else if (cmd == "PASS")
		pass(fd, input);
	else if (cmd == "NICK" && getClient(fd)->isConnected() == true)
		nick(fd, input);
	else if ((cmd == "USER" || cmd == "USERHOST") && getClient(fd)->isConnected() == true)
		user(fd, input);
	else if (cmd == "QUIT")
		quit(fd);
	else if (cmd == "JOIN")
			join(fd, input);
	else if (cmd == "PING")
		pong(fd, input[1]);
	else if (cmd == "PART")
		part(fd);
	else if (cmd == "KICK")
		kick(fd, input);
	else if (cmd == "INVITE")
		invite(fd, input);
	else if (cmd == "MODE")
	{
		std::cout << "tests " << input[0] << std::endl;
		mode(fd, input);
	}
	else if (cmd == "TOPIC")
		topic(fd, input);
	else if (cmd == "MSG")
		msg(fd, input);
	else
		broadcastToChannel(fd, constructMessage(fd, buff));
}

std::string	Server::constructMessage(const int& fd, const char *buff)
{
	std::string message;
	message += '<';
	getClient(fd)->isAdmin() ? message += '@' : message += ' ';
	getClient(fd)->getNick().empty() ? message += getClient(fd)->getFd() : message += getClient(fd)->getNick();
	message += "> ";
	for (size_t i = 0 ; i < strlen(buff) ; i++)
		message += buff[i];
	return message;
}

void	Server::broadcastToChannel(const int& fd, const std::string& message)
{
	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
	if (channel != _channels.end())
		channel->sendMessage(message);
}

// int Server::ParseData(int fd, char *buff)
// {
// 	std::vector<std::string> input = splitInput(buff);
// 	std::cout << "buff : " << buff << std::endl;
// 	// if (getClient(fd)->getInterface() == IRSSI && input[0][0] != '/')
// 	// 	input[0] = "/" + input[0];
// 	if (input[0][0] == '/')
// 		handleCmd(fd, input);
// 	else
// 		broadcastToChannel(fd, constructMessage(fd, buff));
// 	return (0);
// }

// int	detect_irssi(char *buff)
// {
// 	std::string str;
// 	std::string str2;
// 	str2 = "CAP LS";
// 	int i = 0;
// 	while (buff[i] != '\n')
// 	{
// 		str[i] = buff[i];
// 		i++;
// 	}
// 	if (str.find(str2, 0))
// 		return (1);
// 	return (0);
// }

void Server::ReceiveDataClient(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);

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
		std::cout << "buffer: " << buff << std::endl;
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
	for(size_t i = 0; i < this->fds.size(); i++)
	{
		if (this->fds[i].fd == fd)
		{
			this->fds.erase(this->fds.begin() + i);
			break;
		}
	}
	for(size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].getFd() == fd)
		{
			this->clients.erase(this->clients.begin() + i); 
			break;
		}
	}
}

void	reply(int fd, std::string code, std::string msg)
{
	std::string response = ":localhost " + code;
	response += " yrio" + msg + "\r\n";
	send(fd, response.c_str(), response.length(), 0);
}
