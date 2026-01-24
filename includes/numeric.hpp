#ifndef NUMERIC_HPP
#define NUMERIC_HPP
#include <iostream>
namespace IRC {
// NOT OFFICIAL IRC NUMERIC REPLY BUT TO INDICATE DO NOT SEND ANY MESSAGE
const std::string DO_NOTHING = "-01";

// Success replies
const std::string RPL_WELCOME = "001";
const std::string RPL_YOURHOST = "002";
const std::string RPL_CREATED = "003";
const std::string RPL_MYINFO = "004";
const std::string RPL_UMODEIS = "221";
const std::string RPL_CHANNELMODEIS = "324";
const std::string RPL_UNIQOPIS = "325";
const std::string RPL_NOTOPIC = "331";
const std::string RPL_TOPIC = "332";
const std::string RPL_INVITING = "341";
const std::string RPL_INVITELIST = "346";
const std::string RPL_ENDOFINVITELIST = "347";
const std::string RPL_EXCEPTLIST = "348";
const std::string RPL_ENDOFEXCEPTLIST = "349";
const std::string RPL_NAMREPLY = "353";
const std::string RPL_ENDOFNAMES = "366";
const std::string RPL_BANLIST = "367";
const std::string RPL_ENDOFBANLIST = "368";
const std::string RPL_YOUREOPER = "381";
const std::string RPL_YOURESERVICE = "383";

// Error replies
const std::string ERR_NOSUCHNICK = "401";
const std::string ERR_NOSUCHCHANNEL = "403";
const std::string ERR_TOOMANYCHANNELS = "405";
const std::string ERR_TOOMANYTARGETS = "407";
const std::string ERR_UNKNOWNCOMMAND = "421";
const std::string ERR_NONICKNAMEGIVEN = "431";
const std::string ERR_ERRONEUSNICKNAME = "432";
const std::string ERR_NICKNAMEINUSE = "433";
const std::string ERR_NICKCOLLISION = "436";
const std::string ERR_UNAVAILRESOURCE = "437";
const std::string ERR_NOTONCHANNEL = "442";
const std::string ERR_NOTREGISTERED = "451";
const std::string ERR_NEEDMOREPARAMS = "461";
const std::string ERR_ALREADYREGISTRED = "462";
const std::string ERR_PASSWDMISMATCH = "464";
const std::string ERR_KEYSET = "467";
const std::string ERR_CHANNELISFULL = "471";
const std::string ERR_INVITEONLYCHAN = "473";
const std::string ERR_BANNEDFROMCHAN = "474";
const std::string ERR_BADCHANNELKEY = "475";
const std::string ERR_BADCHANMASK = "476";
const std::string ERR_NOCHANMODES = "477";
const std::string ERR_CHANOPRIVSNEEDED = "482";
const std::string ERR_RESTRICTED = "484";
const std::string ERR_NOOPERHOST = "491";
const std::string ERR_UMODEUNKNOWNFLAG = "501";
const std::string ERR_USERSDONTMATCH = "502";
const std::string ERR_USERNOTINCHANNEL = "441";
const std::string ERR_USERONCHANNEL = "443";
const std::string ERR_TOOMANYMATCHES = "00000";  // Does not exist on RFC 2812
}  // namespace IRC

#endif