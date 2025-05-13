#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <vector>
#include <poll.h>
#include <cctype>
#include <cstdlib>
#include "Client.hpp"
#include "Colors.h"
#include "Channel.hpp"

class	Server
{
	private:
		int Port;
		int ServSocket;
		static bool Signal;
		char *Mdp;
		std::vector<Client> clients;
		std::vector<struct pollfd> fds;
		std::vector<Channel>	_channels;

	public:
		Server() { ServSocket = -1; }
		~Server(void) { CloseFds(); }

		std::vector<Client>::iterator	getClient(const int& fd) { return std::find(clients.begin(), clients.end(), fd); }
		std::vector<Client>::iterator	getClient(const std::string& userName) { return std::find(clients.begin(), clients.end(), userName); }
		std::vector<Channel>::iterator	getChannel(const std::string& channel) { return std::find(_channels.begin(), _channels.end(), channel); }

		void ServerInit(int port, char *mdp);
		void SerSocket();
		void	messageFromServer(const int& fd, const std::string& message) const { send(fd, message.c_str(), message.size(), 0); }
		void AcceptIncomingClient();
		void ReceiveDataClient(int fd);
		int	ParseData(int fd, char *buff);
		void	handleCmd(const int& fd, const std::vector<std::string>& input);
		void	nick(const int& fd, const std::vector<std::string>& input);
		void	user(const int& fd, const std::vector<std::string>& input);
		void	quit(const int& fd);
		void	pass(const int& fd, const std::vector<std::string>& input);
		void	join(const int& fd, const std::vector<std::string>& input);
		void	part(const int& fd);
		void	kick(const int& fd, const std::vector<std::string>& usersToKick);
		void	invite(const int& fd, const std::vector<std::string>& usersToInvite);
		void	topic(const int& fd, const std::vector<std::string>& input);
		void	msg(const int& fd, const std::vector<std::string>& input);
		void	list(const int& fd);
		std::string	constructMessage(const int& fd, const char *buff);
		void broadcastToChannel(const int& fd, const std::string& message);

		void	mode(const int&fd, const std::vector<std::string>& input);
		void	parseModes(const int& fd, const std::vector<std::string>& input);
		void	checkModes(const int& fd, std::string str, const std::vector<std::string> input);
		void	addInvite(const int& fd);
		void	addTopicRestriction(const int& fd);
		void	addPassword(const int& fd, std::vector<std::string>& input);

		static void SignalHandler(int signum);
	
		void CloseFds();
		void ClearClients(int fd);

};
