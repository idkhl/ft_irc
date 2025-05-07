#pragma once

#include <iostream>
#include <vector>

class Client
{
	private:
		std::string			_nick;
		std::string			_user;
		bool				connected;
		bool				_admin;
		int				Fd;
		std::string			IPadd;
		bool				_allowed;
		std::string			_channel;
		std::vector<std::string>	_invites;

	public:
						Client(const int& fd, const in_addr& sin_addr) : connected(false), _admin(false), Fd(fd), IPadd(inet_ntoa(sin_addr)), _allowed(false) {std::cout << "coucou" << std::endl;}
						~Client(void) {std::cout << "bye" << std::endl;}

		const int& 			getFd(void) const { return Fd; }
		const std::string&		getNick(void) const { return _nick; }
		const std::string&		getUser(void) const { return _user; }
		std::string&			getChannel(void) { return _channel; }

		const bool&			isConnected(void) const { return connected; }
		const bool&			isAdmin(void) const { return _admin; }
		const bool&			isAllowed(void) const { return _allowed; }
		bool				isInvitedIn(const std::string& channel) const { return std::find(_invites.begin(), _invites.end(), channel) != _invites.end(); }

		void				setFd(int fd){Fd = fd;}
		void				setNick(const std::string& nick) { _nick = nick; }
		void				setUser(const std::string& user) { _user = user; }
		void				setIpAdd(std::string ipadd) { IPadd = ipadd; }
		void				setAuthorization(const bool& allowed) { _allowed = allowed; }
		void				setAdmin(const bool& admin) { _admin = admin; }
		void				setChannel(const std::string& channel) { _channel = channel; }
		void				setConnexion(const bool& connected) { this->connected = connected; }

		bool 				operator==(const Client& client) const { return Fd == client.Fd; }
		bool 				operator==(const int& fd) const { return Fd == fd; }
		bool				operator==(const std::string& userName) const { return _user == userName; }

		void				addInvitation(const std::string& channel) { _invites.push_back(channel); }
};