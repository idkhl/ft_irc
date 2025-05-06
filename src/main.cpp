#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "../include/Server.hpp"

int main(int ac, char **av)
{
	if (ac == 3)
	{
		Server ser;
		std::cout << "--- SERVER ---" << std::endl;
		try
		{
			signal(SIGQUIT, Server::SignalHandler);
			signal(SIGINT, Server::SignalHandler);
			ser.ServerInit(atoi(av[1]), av[2]);
		}
		catch(const std::exception& e)
		{
			ser.CloseFds();
			std::cerr << e.what() << std::endl;
		}
		std::cout << "The Server Closed!" << std::endl;
	}
	else
		std::cout << "Valid Args : './ircserv <port> <password>'\n";
}
