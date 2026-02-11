#include "numeric.hpp"

#include <map>
#include <string>

namespace IRC {

// Initialize the numeric to code string map based on RFC 2812
static void initNumericMap(std::map<Numeric, std::string>& map) {
  // Custom internal codes
  map[DO_NOTHING] = "-1";
  map[RPL_STRREPLY] = "-2";
  map[ERR_REGISTERBEFOREPASS] = "-3";

  // Command responses - RFC 2812 Section 5.1
  map[RPL_WELCOME] = "001";
  map[RPL_YOURHOST] = "002";
  map[RPL_CREATED] = "003";
  map[RPL_MYINFO] = "004";
  map[RPL_UMODEIS] = "221";
  map[RPL_CHANNELMODEIS] = "324";
  map[RPL_UNIQOPIS] = "325";
  map[RPL_NOTOPIC] = "331";
  map[RPL_TOPIC] = "332";
  map[RPL_INVITING] = "341";
  map[RPL_INVITELIST] = "346";
  map[RPL_ENDOFINVITELIST] = "347";
  map[RPL_EXCEPTLIST] = "348";
  map[RPL_ENDOFEXCEPTLIST] = "349";
  map[RPL_NAMREPLY] = "353";
  map[RPL_ENDOFNAMES] = "366";
  map[RPL_BANLIST] = "367";
  map[RPL_ENDOFBANLIST] = "368";
  map[RPL_YOUREOPER] = "381";
  map[RPL_YOURESERVICE] = "383";

  // Error replies - RFC 2812 Section 5.2
  map[ERR_NOSUCHNICK] = "401";
  map[ERR_NOSUCHCHANNEL] = "403";
  map[ERR_TOOMANYCHANNELS] = "405";
  map[ERR_TOOMANYTARGETS] = "407";
  map[ERR_NORECIPIENT] = "411";
  map[ERR_NOTEXTTOSEND] = "412";
  map[ERR_UNKNOWNCOMMAND] = "421";
  map[ERR_NONICKNAMEGIVEN] = "431";
  map[ERR_ERRONEUSNICKNAME] = "432";
  map[ERR_NICKNAMEINUSE] = "433";
  map[ERR_NICKCOLLISION] = "436";
  map[ERR_UNAVAILRESOURCE] = "437";
  map[ERR_USERNOTINCHANNEL] = "441";
  map[ERR_NOTONCHANNEL] = "442";
  map[ERR_USERONCHANNEL] = "443";
  map[ERR_NOTREGISTERED] = "451";
  map[ERR_NEEDMOREPARAMS] = "461";
  map[ERR_ALREADYREGISTRED] = "462";
  map[ERR_PASSWDMISMATCH] = "464";
  map[ERR_KEYSET] = "467";
  map[ERR_CHANNELISFULL] = "471";
  map[ERR_INVITEONLYCHAN] = "473";
  map[ERR_BANNEDFROMCHAN] = "474";
  map[ERR_BADCHANNELKEY] = "475";
  map[ERR_BADCHANMASK] = "476";
  map[ERR_NOCHANMODES] = "477";
  map[ERR_CHANOPRIVSNEEDED] = "482";
  map[ERR_RESTRICTED] = "484";
  map[ERR_NOOPERHOST] = "491";
  map[ERR_UMODEUNKNOWNFLAG] = "501";
  map[ERR_USERSDONTMATCH] = "502";
  map[ERR_TOOMANYMATCHES] = "999";
}

std::string numericToCode(Numeric numeric) {
  static std::map<Numeric, std::string> numericMap;

  // Lazy initialization
  if (numericMap.empty()) {
    initNumericMap(numericMap);
  }

  std::map<Numeric, std::string>::const_iterator it = numericMap.find(numeric);
  if (it != numericMap.end()) {
    return it->second;
  }

  // Return "000" for unknown numeric codes
  return "000";
}

}  // namespace IRC
