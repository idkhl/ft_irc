#include "../include/Client.hpp"
#include "../include/Server.hpp"

void Client::setAuthorization(int fd, const bool& allowed)
{
    if (_allowed == false)
    {
        reply(fd, RPL_WELCOME, " :Welcome to the IRC server");
        reply(fd, RPL_YOURHOST, " :Your host is localhost");
        reply(fd, RPL_CREATED, " :This server was created today");
    }
    _allowed = allowed; 
}
