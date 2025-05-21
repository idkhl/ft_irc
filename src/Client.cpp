#include "../include/Client.hpp"
#include "../include/Server.hpp"

void Client::setAuthorization(int fd, const bool& allowed)
{
    if (_allowed == false)
    {
        (void)fd;
        	// std::string response = ":localhost CAP LS :\r\n";
	        // send(fd, response.c_str(), response.length(), 0);
        	reply(fd, RPL_WELCOME, " :Welcome to the IRC server");
			reply(fd, RPL_YOURHOST, " :Your host is localhost");
			reply(fd, RPL_CREATED, " :This server was created today");
            // reply(fd, RPL_MYINFO, " :localhost 0.1 i o");
            // reply(fd, "375", " :-localhost Message of the Day -");
            // reply(fd, "372", " :- Bienvenue sur ton propre serveur IRC !");
            // reply(fd, "376", " :End of /MOTD command.");
    }
    _allowed = allowed; 
}
