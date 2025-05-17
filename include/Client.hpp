#pragma once

#include <iostream>
#include <vector>
#include "Server.hpp"
#include "Channel.hpp"

#define NC 1
#define IRSSI 0

class Client
{
	private:
		std::string			_nick;
		std::string			_user;
		bool				connected;
		int				Fd;
		int				interface;
		std::string			IPadd;
		std::vector<std::string>	_invites;
		std::vector<std::string>	_channels;
		bool				_allowed;

	public:
						Client(const int& fd, const in_addr& sin_addr) : connected(false), Fd(fd), IPadd(inet_ntoa(sin_addr)), _allowed(false) {}
						~Client(void) {}

		const int& 			getFd(void) const { return Fd; }
		const std::string&		getNick(void) const { return _nick; }
		const std::string&		getUser(void) const { return _user; }
		const std::vector<std::string>&	getChannels(void) const { return _channels; }
		int				getInterface(void) const { return interface; }

		const bool&			isConnected(void) const { return connected; }
		bool				isInvitedIn(const std::string& channel) const { return std::find(_invites.begin(), _invites.end(), channel) != _invites.end(); }
		bool				isAdmin(const Channel& channel) const { return std::find(channel.getAdmins().begin(), channel.getAdmins().end(), Fd) == channel.getAdmins().end() ? false : true; }
		bool				isInChannel(const std::string& channelName) const { return std::find(_channels.begin(), _channels.end(), channelName) != _channels.end() ? true : false; }

		void				setInterface(int interface) { this->interface = interface; }
		void				setNick(const std::string& nick) { _nick = nick; }
		void				setUser(const std::string& user) { _user = user; }
		void				setIpAdd(std::string ipadd) { IPadd = ipadd; }
		void				setAuthorization(const int& fd, const bool& allowed);
		void				setConnexion(const bool& connected) { this->connected = connected; }

		bool 				operator==(const Client& client) const { return Fd == client.Fd; }
		bool 				operator==(const int& fd) const { return Fd == fd; }
		bool				operator==(const std::string& userName) const { return _user == userName; }

		void				addInvitation(const std::string& channel) { _invites.push_back(channel); }
		void				deleteInvitation(const std::string& channelName) { if (isInChannel(channelName)) _invites.erase(std::find(_invites.begin(), _invites.end(), channelName)); }
		void				addChannel(const std::string& channelName) { if (!isInChannel(channelName)) _channels.push_back(channelName); }
		void				deleteChannel(const std::string& channelName) { if (isInChannel(channelName)) _channels.erase(std::find(_channels.begin(), _channels.end(), channelName)); }
};