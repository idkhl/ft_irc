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
#include <bits/stdc++.h>
#include "Client.hpp"
#include "Colors.h"
#include "Channel.hpp"


#define RPL_WELCOME "001"
#define RPL_YOURHOST "002"
#define RPL_CREATED "003"
#define RPL_MYINFO "004"

#define RPL_UMODEIS "221"
#define RPL_ADMINME "256"
#define RPL_CHANNELMODEIS "324"
#define RPL_CREATIONTIME "329"
#define RPL_NOTOPIC "331"
#define RPL_TOPIC "332"
#define RPL_TOPICWHOTIME "333"
#define RPL_INVITELIST "336"
#define RPL_INVITING "341"
#define RPL_NAMREPLY "353"
#define RPL_ENDOFNAMES "366"

#define ERR_UNKNOWNERROR "400"
#define ERR_NOSUCHNICK "401"
#define ERR_NOSUCHCHANNEL "403"
#define ERR_CANNOTSENDTOCHAN "404"
#define ERR_TOOMANYCHANNELS "405"
#define ERR_NOORIGIN "409"
#define ERR_NORECIPIENT "411"
#define ERR_NOTEXTTOSEND "412"
#define ERR_UNKNOWNCOMMAND "421"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_USERNOTINCHANNEL "441"
#define ERR_NOTONCHANNEL "442"
#define ERR_USERONCHANNEL "443"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTERED "462"
#define ERR_PASSWDMISMATCH "464"
#define ERR_KEYSET "467"
#define ERR_CHANNELISFULL "471"
#define ERR_UNKNOWNMODE "472"
#define ERR_INVITEONLYCHAN "473"
#define ERR_BADCHANNELKEY "475"
#define ERR_BADCHANMASK "476"
#define ERR_NOPRIVILEGES "481"
#define ERR_CHANOPRIVSNEEDED "482"

#define ERR_UMODEUNKNOWMFLAG "501"
#define ERR_USERSDONTMATCH "502"

class Client;
class Channel;

class	Server
{
	private:
		int 				Port;
		int 				ServSocket;
		static bool			Signal;
		char 				*Mdp;
		std::vector<Client> 		clients;
		std::vector<struct pollfd> 	fds;
		std::vector<Channel>		_channels;

	public:
						Server() { ServSocket = -1; }
						~Server(void) { CloseFds(); }

		std::vector<Client>::iterator	getClient(const int& fd) { return std::find(clients.begin(), clients.end(), fd); }
		std::vector<Client>::iterator	getClient(const std::string& nickname) { return std::find(clients.begin(), clients.end(), nickname); }
		std::vector<Channel>::iterator	getChannel(const std::string& channel) { return std::find(_channels.begin(), _channels.end(), channel); }
		std::vector<std::string>	getUserInput(const int& fd);
		size_t				getNbrChannel(void) const { return _channels.size(); }

		void				ServerInit(int port, char *mdp);
		void				SerSocket();
		void				messageFromServer(const int& fd, const std::string& message) const { send(fd, message.c_str(), message.size(), 0); }
		void				AcceptIncomingClient();
		void				ReceiveDataClient(int fd);
		int				ParseData(int fd, char *buff);
		void				handleCmd(const int& fd, const char *buff);
		void				nick(const int& fd, const std::vector<std::string>& input, int index);
		void				user(const int& fd, const std::vector<std::string>& input, int index);
		void				quit(const int& fd);
		void				pass(const int& fd, const std::vector<std::string>& input, int index);
		void				join(const int& fd, const std::vector<std::string>& input);
		void				part(const int& fd);
		void				pong(const int& fd, std::string token);
		void				kick(const int& fd, const std::vector<std::string>& usersToKick);
		void				invite(const int& fd, const std::vector<std::string>& usersToInvite);
		void				topic(const int& fd, const std::vector<std::string>& input);
		void				msg(const int& fd, const std::vector<std::string>& input);
		void				broadcastToChannel(const std::vector<std::string>& input);
		void				mode(const int&fd, const std::vector<std::string>& input);
		void				parseModes(const int& fd, const std::vector<std::string>& input, const std::string& channelName);
		void				checkModes(const int& fd, std::string str, std::vector<std::string> input, const std::string& channelName);
		void				addInvite(const int& fd, char sign, const std::string& channelName);
		void				addTopicRestriction(const int& fd, char sign, const std::string& channelName);
		void				addPassword(const int& fd, char sign, const std::string& channelName, std::vector<std::string>& input);
		int				checkChannelPassword(const int& fd, std::string channel, const std::vector<std::string>& input);
		void				addOperator(char sign, const std::string& channelName, const int& fd, std::vector<std::string>& input);
		void				addUserLimit(const int& fd, char sign, const std::string& channelName, std::vector<std::string>& input);
		void				check_connexion(const int& fd, std::vector<std::string> input);

		void				deleteChannel(const std::string& channelName);
		void				deleteChannel(std::vector<Channel>::iterator channel) { _channels.erase(channel); }

		static void			SignalHandler(int signum);

		void				CloseFds();
		void				ClearClients(int fd);

};

void	reply(std::vector<Client>::iterator client, std::string code, std::string msg);
void	sendToIrssi(std::vector<Client>::iterator client, std::string message);