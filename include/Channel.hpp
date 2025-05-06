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
#include "tools.hpp"

class	Channel
{
	private:
		std::vector<Client>		_clients;
		std::string			_name;
		std::vector<pollfd>		_pollfds;
		std::vector<int>		_adminFds;

	public:
						Channel(Client& client, const std::string& name);
						~Channel(void) {}
						Channel(const Channel& channel) : _clients(channel._clients), _name(channel._name) {}

		Channel&			operator=(const Channel& channel) { (void)channel; return *this; }
		bool				operator==(const std::string& name) const { return _name == name ? true : false; }
		bool				operator!=(const std::string& name) const { return _name != name ? true : false; }

		const std::string&		getName(void) const { return _name; }
		std::vector<Client>::iterator	getClient(const int& fd) { return std::find(_clients.begin(), _clients.end(), fd); }
		const std::vector<int>&		getAdmins(void) const { return _adminFds; }

		void				setName(const std::string& name) { _name = name; }

		void				sendMessage(const std::string& message) const;
		void				join(Client& client);
		void				deleteClient(const int& fd);
};