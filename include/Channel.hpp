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
#include <cstring>
#include <cstdlib>
#include "Client.hpp"
#include "Colors.h"

class	Channel
{
	private:
		std::vector<Client *>	_clients;
		std::string				_name;
		std::vector<int>		_adminFds;
		std::string				_topic;
		std::vector<Client>		_invited;
		std::string				_password;
		bool					_inviteMode;
		bool					_topicRestriction;
		size_t					_clientLimit;

	public:
						Channel(Client& client, const std::string& name);
						~Channel(void) {}

		bool				operator==(const std::string& name) const { return _name == name ? true : false; }
		bool				operator!=(const std::string& name) const { return _name != name ? true : false; }

		const std::string&			getName(void) const { return _name; }
		Client						*getClient(const int& fd);
		const std::vector<int>&		getAdmins(void) const { return _adminFds; }
		const std::string&		getTopic(void) const { return _topic; }
		
		const bool&			isInviteOnly(void) const { return _inviteMode; }
		const bool&			isTopicRestriction(void) const { return _topicRestriction; }
		std::string&				getPassword() {return _password;}
		size_t 						getClientCount() const { return _clients.size(); }
		size_t						getClientLimit(void) {return _clientLimit;}

		void				setName(const std::string& name) { _name = name; }
		void				setInviteMode(bool mode) { _inviteMode = mode; }
		void				setTopicRestriction(bool mode) { _topicRestriction = mode; };
		void 				setPassword(std::string pass) { _password = pass; }
		void				setTopic(const std::string& topic) { _topic = topic; }

		void				setClientLimit(size_t limit) {_clientLimit = limit;}
		void				sendMessage(const std::string& message) const;
		void				join(Client& client);
		void				deleteClient(const int& fd);
};