#ifndef NUMERIC_HPP
#define NUMERIC_HPP
#include <iostream>
namespace IRC {
    // Success replies
    const std::string RPL_WELCOME = "001";
    const std::string RPL_YOURHOST = "002";
    const std::string RPL_CREATED = "003";
    const std::string RPL_MYINFO = "004";
    
    // Error replies
    const std::string ERR_NOSUCHNICK = "401";
    const std::string ERR_NOSUCHCHANNEL = "403";
    const std::string ERR_UNKNOWNCOMMAND = "421";
    const std::string ERR_NONICKNAMEGIVEN = "431";
    const std::string ERR_NICKNAMEINUSE = "433";
    const std::string ERR_NOTREGISTERED = "451";
    const std::string ERR_NEEDMOREPARAMS = "461";
    const std::string ERR_PASSWDMISMATCH = "464";
}

#endif