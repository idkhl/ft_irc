#pragma once

#include <iostream>

class Client
{
	private:
		std::string		_nick;
		std::string		_user;
		bool			connected;
		bool			_admin;
		int			Fd;
		std::string		IPadd;
		bool			_allowed;
		std::string		_channel;

	public:
					Client(const int& fd, const in_addr& sin_addr) : connected(false), _admin(false), Fd(fd), IPadd(inet_ntoa(sin_addr)), _allowed(false) {}
					~Client(void) {}

		const int& 		getFd(void) const { return Fd; }
		const std::string&	getNick(void) const { return _nick; }
		const std::string&	getUser(void) const { return _user; }
		std::string&		getChannel(void) { return _channel; }

		const bool&		isConnected(void) const { return connected; }
		const bool&		isAdmin(void) const { return _admin; }
		const bool&		isAllowed(void) const { return _allowed; }

		void			setFd(int fd){Fd = fd;}
		void			setNick(const std::string& nick) { _nick = nick; }
		void			setUser(const std::string& user) { _user = user; }
		void			setIpAdd(std::string ipadd) { IPadd = ipadd; }
		void			setAuthorization(const bool& allowed) { _allowed = allowed; }
		void			setAdmin(const bool& admin) { _admin = admin; }
		void			setChannel(const std::string& channel) { _channel = channel; }

		bool 			operator==(const Client& client) const { return Fd == client.Fd; }
		bool 			operator==(const int& fd) const { return Fd == fd; }
};