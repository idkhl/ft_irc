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
#include <list>
#include <vector>
#include <poll.h>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include "Colors.h"

class Client;

class	Channel
{
	private:
		std::list<Client *>		_clients;
		std::string			_name;
		std::vector<int>		_adminFds;
		std::string			_topic;
		std::vector<Client>		_invited;
		std::string			_password;
		bool				_inviteMode;
		bool				_topicRestriction;
		int				_clientLimit;

	public:
						Channel(Client& client, const std::string& name);
						~Channel(void) {}

		bool				operator==(const std::string& name) const { return _name == name ? true : false; }
		bool				operator!=(const std::string& name) const { return _name != name ? true : false; }

		const std::string&		getName(void) const { return _name; }
		Client				*getClient(const int& fd);
		Client				*getClient(const std::string& userName);
		const std::vector<int>&		getAdmins(void) const { return _adminFds; }
		Client				*getAdmin(const int& fd);
		Client				*getAdmin(const std::string& userName);
		size_t				getNbrAdmins(void) const { return _adminFds.size(); }
		const std::string&		getTopic(void) const { return _topic; }
		const std::list<Client *>     &getClients() const {return _clients;}
		std::string&			getPassword() {return _password;}
		size_t 				getClientCount() const { return _clients.size(); }
		int&				getClientLimit(void) {return _clientLimit;}
		
		const bool&			isInviteOnly(void) const { return _inviteMode; }
		const bool&			isTopicRestriction(void) const { return _topicRestriction; }

		void				setName(const std::string& name) { _name = name; }
		void				setInviteMode(bool mode) { _inviteMode = mode; }
		void				setTopicRestriction(bool mode) { _topicRestriction = mode; };
		void 				setPassword(std::string pass) { _password = pass; }
		void				setTopic(const std::string& topic) { _topic = topic; }
		void				setClientLimit(int limit) {_clientLimit = limit;}
		void				sendMessage(const std::string& message) const;

		void				addClient(Client& client);
		void 				addAdmin(int fd) { if (std::find(_adminFds.begin(), _adminFds.end(), fd) == _adminFds.end()) _adminFds.push_back(fd); }
		void				deleteClient(const int& fd);
		void				deleteAdmin(const int& fd);
		void				delegatePower(void);
};