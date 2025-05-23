#include "../include/Client.hpp"
#include "../include/Server.hpp"

void    Client::setAuthorization(const int& fd, const bool& allowed)
{
    if (_allowed == false)
    {
        std::string msg = getUser() + " :Welcome to the Internet Relay Network " + getNick() + "!" + getUser() + "@localhost";
        // std::string msg = "001 idakhlao :Welcome to the IRC Network idakhlao!idakhlao@localhost\n";
        // std::cout << msg << std::endl;
        reply(fd, RPL_WELCOME, msg);

        // reply(fd, RPL_WELCOME, " :Welcome to the IRC server");
        // reply(fd, RPL_YOURHOST, " :Your host is localhost");
        // reply(fd, RPL_CREATED, " :This server was created today");
    }
    _allowed = allowed;
}