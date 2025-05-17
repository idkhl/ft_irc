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
	messageFromServer(incofd, "WELCOME IN FT_IRC!\nEnter your USERNAME, NICKNAME and PASSWORD\n");
}

void	Server::handleCmd(const int& fd, const std::vector<std::string>& input)
{
	std::string cmd = input[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), toupper);
	if (cmd == "/CAP")
		return;
	else if (cmd == "/PASS")
		pass(fd, input);
	else if (cmd == "/NICK" && getClient(fd)->isConnected() == true)
		nick(fd, input);
	else if (cmd == "/USER" && getClient(fd)->isConnected() == true)
		user(fd, input);
	else if (cmd == "/QUIT")
		quit(fd);
	else if (cmd == "/JOIN")
	{
		std::string channelName = join(fd, input);
		if (channelName.empty())
			return;

		if (!getChannel(channelName)->getPassword().empty())
		{
			messageFromServer(fd, std::string("Please enter " + channelName + "'s password:\n"));
			std::vector<std::string> passwordInput = getUserInput(fd);
			if (passwordInput.empty() || checkChannelPassword(fd, channelName, passwordInput) == -1)
			{
				messageFromServer(fd, "Failed to join the channel. Incorrect password.\n");
				return;
			}
		}
	}
	else if (cmd == "/KICK")
		kick(fd, input);
	else if (cmd == "/INVITE")
		invite(fd, input);
	else if (cmd == "/MODE")
		mode(fd, input);
	else if (cmd == "/TOPIC")
		topic(fd, input);
	else if (cmd == "/MSG")
		msg(fd, input);
	else if (cmd == "/LIST")
		list(fd);
	else if (cmd == "/HELP")
		help(fd);
}

std::string	Server::constructMessage(const int& fd, const char *buff)
{
	std::string message;
	message += '<';
	// getClient(fd)->isAdmin(*getChannel(getClient(fd)->getChannel())) ? message += '@' : message += ' ';
	getClient(fd)->getNick().empty() ? message += getClient(fd)->getFd() : message += getClient(fd)->getNick();
	message += "> ";
	for (size_t i = 0 ; i < strlen(buff) ; i++)
		message += buff[i];
	return message;
}

// void	Server::broadcastToChannel(const int& fd, const std::string& message)
// {
// 	std::vector<Channel>::iterator channel = getChannel(getClient(fd)->getChannel());
// 	if (channel != _channels.end())
// 		channel->sendMessage(message);
// }

static void	slideToTheLeft(std::string& str, const size_t& index)
{
	for (size_t i = index ; i < str.size() - 1 ; i++)
		str[i] = str[i + 1];
}

static std::vector<std::string>	splitInput(std::string str)
{
	bool flag = false;
	char inQuote = 0;
	for (size_t i = 0 ; i < str.size() ; i++)
	{
		if (str[i] == '\n')
			str[i] = ' ';
	}
	for (size_t i = 0 ; i < str.size() ; i++)
	{
		if (!inQuote && (str[i] == '\'' || str[i] == '"'))
		{
			inQuote = str[i];
			slideToTheLeft(str, i);
		}
		else if (inQuote && str[i] == inQuote)
		{
			inQuote = 0;
			slideToTheLeft(str, i);
		}
		if (str[i] != ' ')
			flag = true;
		if (str[i] == ' ' && flag && !inQuote)
			str[i] = 0;
	}
	std::vector<std::string> result;
	for (size_t i = 0 ; i < str.size() ; i++)
	{
		if (i == 0 && str[i])
			result.push_back(std::string(&str[i]));
		else if (str[i] && !str[i - 1])
			result.push_back(std::string(&str[i]));
	}
	return result;
}

std::vector<std::string> Server::getUserInput(const int& fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    int pollResult = poll(&pfd, 1, 5000);
    if (pollResult <= 0)
    {
        std::cout << "Timeout or error while waiting for input from client <" << fd << ">." << std::endl;
        return std::vector<std::string>();
    }

    ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        std::cout << "Failed to receive input from client <" << fd << ">." << std::endl;
        return std::vector<std::string>();
    }

    buffer[bytes] = '\0';
    std::cout << "Received input from client <" << fd << ">: " << buffer << std::endl;
    return splitInput(std::string(buffer));
}

int	Server::ParseData(int fd, char *buff)
{
	std::vector<std::string> cmds;
	cmds.push_back("/PASS");
	cmds.push_back("/NICK");
	cmds.push_back("/USER");
	cmds.push_back("/JOIN");
	std::vector<std::string> input = splitInput(buff);
	std::cout << "buff : " << buff << std::endl;
	if (getClient(fd)->getInterface() == IRSSI && input[0][0] != '/')
		input[0] = "/" + input[0];
	std::cout << "input : " << input[0] << std::endl;
	if (std::find(cmds.begin(), cmds.end(), input[0]) != cmds.end())
		handleCmd(fd, input);
	else
		broadcastToChannel(fd, constructMessage(fd, buff));
	return (0);
}

int	detect_irssi(char *buff)
{
	std::string str;
	std::string str2;
	str2 = "CAP LS";
	int i = 0;
	while (buff[i] != '\n')
	{
		str[i] = buff[i];
		i++;
	}
	if (str.find(str2, 0))
		return (1);
	return (0);
}

void Server::ReceiveDataClient(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHITE << std::endl;
		ClearClients(fd);
		close(fd);
	}
	else
	{
		buff[bytes] = '\0';
		// std::cout << buff;
		if (getClient(fd)->getInterface() == NC)
			if (detect_irssi(buff))
				getClient(fd)->setInterface(IRSSI);
		std::string nick;
		//here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
		ParseData(fd, buff);
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
	std::vector<Client>::iterator client = getClient(fd);
	for (size_t i = 0 ; i < client->getChannels().size() ; i++)
	{
		std::vector<Channel>::iterator channel = getChannel(client->getChannels()[i]);
		channel->deleteAdmin(fd);
		channel->deleteClient(fd);
		if (channel->getClientCount() == 0)
		{
			deleteChannel(channel);
			break;
		}
		if (channel->getNbrAdmins() == 0)
		{
			channel->addAdmin(channel->getClients()[0]->getFd());
			messageFromServer(channel->getClients()[0]->getFd(), "You are now an operator of the channel " + channel->getName() + '\n');	
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

void	reply(int fd, std::string code, std::string msg)
{
	std::string response = ":localhost " + code;
	response += " " + msg + "\r\n";
	send(fd, response.c_str(), response.length(), 0);
}