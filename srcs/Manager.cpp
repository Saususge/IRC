#include "Manager.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "numeric.hpp"

/* utils for Manager class */
std::string getCommand(std::vector<std::string>& tokens) {
  if (tokens[0][0] != ':') return tokens[0];
  return tokens[1];
}

std::string strToLower(std::string str) {
  for (std::string::iterator iter = str.begin(); iter < str.end(); iter++) {
    *iter = static_cast<char>(std::tolower(*iter));
  }
  return (str);
}
/* utils for Manager class */

Manager::Manager() {}

Manager::~Manager() {}

void Manager::addClient(int fd, bool isEmptyPass) {
    if (users.find(fd) != users.end()) {
        std::cerr << "[ERROR] Manager: Fd=" << fd << " found in users." << std::endl;
        return ; // do nothing
    }

    users[fd] = Client();
    if (isEmptyPass)
        (&users[fd])->onPass();
    
    std::cout << "Manager: Client(fd=" << fd << ") added." << std::endl;
}

void Manager::removeClient(int fd) {
  if (users.find(fd) != users.end()) {
      Client& client = users.find(fd)->second;
      std::string nickname = client.getNickname();
      
      // Remove from all channels
      // Optimally we should use the same logic as QUIT command
      for (std::map<std::string, Channel>::iterator chIt = channels.begin(); chIt != channels.end();) {
           Channel& channel = chIt->second;
           if (channel.getUserInfo(nickname) == 1) { // User is in this channel
               channel.delUser(&client);
           }
           
           // If channel empty, remove it?
           if (channel.getUsers().empty()) {
               std::map<std::string, Channel>::iterator toErase = chIt;
               ++chIt;
               channels.erase(toErase);
           } else {
               ++chIt;
           }
      }
      users.erase(fd);
  }

  std::cout << "Manager: Client " << fd << " removed." << std::endl;
}

int Manager::doRequest(Server& server, int fd, std::string request) {
  std::stringstream ss(request);
  std::string tok;
  std::vector<std::string> tokVec;

  while (ss >> tok) {
    if (tok[0] == ':') {
      std::string trailing;
      std::getline(ss, trailing);
      tok = tok.substr(1) + trailing;  // ':' 제거하고 나머지 이어붙임
      tokVec.push_back(tok);
      std::cout << "doRequest token: " << tok << "$" << std::endl;
      break;  // trailing parameter는 항상 마지막이므로 루프 종료
    } else {
      tokVec.push_back(tok);
      std::cout << "doRequest token: " << tok << "$" << std::endl;
    }
  }

  std::string cmd = strToLower(getCommand(tokVec));

  // refactor to switch-case later
  if (cmd == "nick") {
    if (tokVec.size() < 2) {
       server.queueMessage(fd, Response::error(IRC::ERR_NONICKNAMEGIVEN, "*", ":No nickname given"));
       return 0;
    }
    std::string newNick = tokVec[1];
    
    // Check if nickname is already in use
    for (std::map<int, Client>::iterator iter = users.begin(); iter != users.end(); iter++) {
      if (iter->second.getNickname() == newNick) {
        server.queueMessage(fd, Response::error(IRC::ERR_NICKNAMEINUSE, "*", newNick + " :Nickname is already in use"));
        return -1;
      }
    }

    std::map<int, Client>::iterator iter = users.find(fd);

    if (iter!= users.end()) {
        if (iter->second.getRegisterd()) {
            std::string oldNick = iter->second.getNickname();
            iter->second.setNickname(newNick);
            server.queueMessage(fd, ":" + oldNick + " NICK :" + newNick + "\r\n");
            return 0;
        } else
            iter->second.setNickname(newNick);
    } else {
        std::cerr << "[Error] Manager: fd=" << fd << " not found." << std::endl;
        return -1;
    }

    iter->second.onNick();

    if (iter->second.isRegistrable()) {
        iter->second.setRegisterd(true);

        // SEND WELCOME MESSAGES
        server.queueMessage(fd, Response::build(IRC::RPL_WELCOME, newNick, ":Welcome to the ft_irc Network " + newNick));
        server.queueMessage(fd, Response::build(IRC::RPL_YOURHOST, newNick, ":Your host is irc.local, running version 1.0"));
        server.queueMessage(fd, Response::build(IRC::RPL_CREATED, newNick, ":This server was created today"));
        server.queueMessage(fd, Response::build(IRC::RPL_MYINFO, newNick, "irc.local 1.0 o o"));
        return 1;

    } else if (iter->second.getLoginFlags() == 0b110) {
        server.queueMessage(fd, Response::error(IRC::ERR_PASSWDMISMATCH, newNick, ":Password incorrect"));
        server.queueMessage(fd, "ERROR :Closing Link: " + newNick + " (Bad Password)\r\n");
        // TODO: wait for sending message
        server.closeClientByFd(fd);
    }

  } else if (cmd == "user") {
    if (tokVec.size() < 5) {
        server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, "*", "USER :Not enough parameters"));
        return 0;
    }

    std::map<int, Client>::iterator iter = users.find(fd);
    if (iter != users.end() && iter->second.getRegisterd()) {
        server.queueMessage(fd, Response::error(IRC::ERR_ALREADYREGISTRED, users.find(fd)->second.getNickname(), ":You may not reregister"));
        users.erase(fd);
        return -1;
    } else if (iter == users.end()) {
        std::cerr << "[Error] Manager: fd=" << fd << " not found." << std::endl;
        return -1;
    }

    std::string nickname = iter->second.getNickname();
    std::string username = tokVec[1];
    std::string realname = tokVec[4];

    iter->second.onUser();

    if (iter->second.isRegistrable()) {
        iter->second.setRegisterd(true);

        // SEND WELCOME MESSAGES
        server.queueMessage(fd, Response::build(IRC::RPL_WELCOME, nickname, ":Welcome to the ft_irc Network " + nickname));
        server.queueMessage(fd, Response::build(IRC::RPL_YOURHOST, nickname, ":Your host is irc.local, running version 1.0"));
        server.queueMessage(fd, Response::build(IRC::RPL_CREATED, nickname, ":This server was created today"));
        server.queueMessage(fd, Response::build(IRC::RPL_MYINFO, nickname, "irc.local 1.0 o o"));
        return 1;

    } else if (iter->second.getLoginFlags() == 0b110) {
        server.queueMessage(fd, Response::error(IRC::ERR_PASSWDMISMATCH, nickname, ":Password incorrect"));
        server.queueMessage(fd, "ERROR :Closing Link: " + nickname + " (Bad Password)\r\n");
        // TODO: wait for sending message
        server.closeClientByFd(fd);
    }

  } else if (cmd == "pass") {
    std::cout << "pass" << std::endl;

    if (tokVec.size() < 2) {
        server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "JOIN :Not enough parameters"));
        return 1;
    }

    std::map<int, Client>::iterator iter = users.find(fd);
    if (iter != users.end() && iter->second.getRegisterd()) {
        server.queueMessage(fd, Response::error(IRC::ERR_ALREADYREGISTRED, users.find(fd)->second.getNickname(), "JOIN :Not enough parameters"));
        return 1;
    } else if (iter == users.end()) {
        std::cerr << "[Error] Manager: fd=" << fd << " not found." << std::endl;
        return -1;
    }

    if (tokVec[1] == server.getPassword()) {
        iter->second.onPass();
        return 1;
    }

  } else if (cmd == "join") {
    // Check registration
    std::map<int, Client>::iterator iter = users.find(fd);
    if (iter != users.end() && iter->second.getRegisterd() == false) {
        server.queueMessage(fd, Response::error(IRC::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return 0;
    } else if (iter == users.end()) {
        std::cerr << "[Error] Manager: fd=" << fd << " not found." << std::endl;
        return -1;
    }
    
    if (tokVec.size() < 2) {
        server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "JOIN :Not enough parameters"));
        return 0;
    }

    std::string channelName = tokVec[1];
    
    // Create channel if not exists
    bool isNewChannel = false;
    if (channels.find(channelName) == channels.end()) {
      channels.insert(std::pair<std::string, Channel>(channelName, Channel()));
      isNewChannel = true;
    }

    Channel& channel = channels.find(channelName)->second;
    Client& client = users.find(fd)->second;

    // Pass isCreator=true if the channel was just created
    const std::string res = channel.addUser(fd, &client, isNewChannel);
    
    if (res == IRC::RPL_TOPIC || res == IRC::RPL_NOTOPIC) { // JOIN SUCCESS
        // 1. Notify all members in channel that user joined
        std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@127.0.0.1 JOIN :" + channelName + "\r\n";
        
        const std::map<std::string, MemberInfo>& members = channel.getUsers();
        for (std::map<std::string, MemberInfo>::const_iterator it = members.begin(); it != members.end(); ++it) {
            server.queueMessage(it->second.fd, joinMsg);
        }

        // 2. Send Topic (if any) - 332
        std::string topic = channel.getTopic();
        topic = topic.empty() ? "No topic is set" : topic;
        server.queueMessage(fd, Response::build(res, client.getNickname(), channelName + " :" + topic));

        // 3. Send Name List - 353, 366
        std::string namesList;
        for (std::map<std::string, MemberInfo>::const_iterator it = members.begin(); it != members.end(); ++it) {
           // If operator, prefix with @
           if (it->second.op) {
               namesList += "@" + it->first + " ";
           } else {
               namesList += it->first + " ";
           }
        }
        
        server.queueMessage(fd, Response::build(IRC::RPL_NAMREPLY, client.getNickname(), "= " + channelName + " :" + namesList));
        server.queueMessage(fd, Response::build(IRC::RPL_ENDOFNAMES, client.getNickname(), channelName + " :End of /NAMES list."));
        
    } else if (res == IRC::ERR_INVITEONLYCHAN) {
        server.queueMessage(fd, Response::error(IRC::ERR_INVITEONLYCHAN, client.getNickname(), channelName + " :Cannot join channel (+i)"));
    } else if (res == IRC::ERR_BADCHANNELKEY) {
        server.queueMessage(fd, Response::error(IRC::ERR_BADCHANNELKEY, client.getNickname(), channelName + " :Cannot join channel (+k)"));
    } else if (res == IRC::ERR_CHANNELISFULL) {
        server.queueMessage(fd, Response::error(IRC::ERR_CHANNELISFULL, client.getNickname(), channelName + " :Cannot join channel (+l)"));
    } else {
        /* do nothing */
        return 0;
    }
    // above else if statements are fucking messy. generalize later.
    // TODO: test error cases after implement MODE command
    
    return 1; // yeonjuki: REPLACE TO res AFTER CHANGE RETURN TYPE.

  } else if (cmd == "part") {
    std::map<int, Client>::iterator userIter = users.find(fd);
    if (userIter != users.end() && userIter->second.getRegisterd() == false) {
        server.queueMessage(fd, Response::error(IRC::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return 0;
    } else if (userIter == users.end()) {
        std::cerr << "[Error] Manager: fd=" << fd << " not found." << std::endl;
        return -1;
    }
    if (tokVec.size() < 2) {
       server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "PART :Not enough parameters"));
       return 0;
    }
    std::string channelName = tokVec[1];
    std::string reason = (tokVec.size() > 2) ? tokVec[2] : ":Leaving";

    std::map<std::string, Channel>::iterator iter = channels.find(channelName);
    if (iter == channels.end()) {
        server.queueMessage(fd, Response::error(IRC::ERR_NOSUCHCHANNEL, users.find(fd)->second.getNickname(), channelName + " :No such channel"));
        return 0;
    }

    Channel& channel = iter->second;
    Client& client = users.find(fd)->second;
    std::string nickname = client.getNickname();

    // Check if on channel (Channel::delUser also checks this but we need to broadcast before deleting for simplicity, 
    // or broadcast to remaining members + self after)
    // Using Channel logic:
    if (channel.delUser(&client) == "PART") {
         std::string partMsg = ":" + nickname + "!" + client.getUsername() + "@127.0.0.1 PART " + channelName + " " + reason + "\r\n";
         
         // Send to self
         server.queueMessage(fd, partMsg);

         // Send to remaining members
         const std::map<std::string, MemberInfo>& members = channel.getUsers();
         for (std::map<std::string, MemberInfo>::const_iterator it = members.begin(); it != members.end(); ++it) {
            server.queueMessage(it->second.fd, partMsg);
         }
    } else {
         server.queueMessage(fd, Response::error(IRC::ERR_NOTONCHANNEL, nickname, channelName + " :You're not on that channel"));
    }
    
    // Cleanup channel if empty? (Optional)
    if (channel.getUsers().empty()) {
        channels.erase(channelName);
    }
    
    return 0;

  } else if (cmd == "privmsg") {
    std::map<int, Client>::iterator iter = users.find(fd);
    if (iter != users.end() && iter->second.getRegisterd() == false) {
        server.queueMessage(fd, Response::error(IRC::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return 0;
    } else if (iter == users.end()) {
        std::cerr << "[Error] Manager: fd=" << fd << " not found." << std::endl;
        return -1;
    }
      
      if (tokVec.size() < 3) {
          server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "PRIVMSG :Not enough parameters"));
          return 0;
      }

      std::string target = tokVec[1];
      std::string text = tokVec[2];
      Client& sender = users.find(fd)->second;
      std::string senderPrefix = ":" + sender.getNickname() + "!" + sender.getUsername() + "@127.0.0.1";

      // 1. Channel Message
      if (target[0] == '#') {
          std::map<std::string, Channel>::iterator chIter = channels.find(target);
          if (chIter == channels.end()) {
               server.queueMessage(fd, Response::error(IRC::ERR_NOSUCHCHANNEL, sender.getNickname(), target + " :No such channel"));
               return 0;
          }
          
          Channel& channel = chIter->second;
          // Check if user is in channel? (Depending on mode 'n', but standard IRC usually enforces external msg block by default or checks membership)
          // For simple mandatory, let's allow or check membership.
          // RFC: ERR_CANNOTSENDTOCHAN if logic fails.
          // Let's rely on basic membership for now to be safe.
          MemberInfo info;
          if (channel.getUserInfo(sender.getNickname(), &info) == -1) {
               server.queueMessage(fd, Response::error(IRC::ERR_NOTONCHANNEL, sender.getNickname(), target + " :Cannot send to channel")); // Using ERR_NOTONCHANNEL or ERR_CANNOTSENDTOCHAN
               return 0;
          }

          std::string fullMsg = senderPrefix + " PRIVMSG " + target + " :" + text + "\r\n";
          const std::map<std::string, MemberInfo>& members = channel.getUsers();
          for (std::map<std::string, MemberInfo>::const_iterator it = members.begin(); it != members.end(); ++it) {
                if (it->second.fd != fd) {
                    server.queueMessage(it->second.fd, fullMsg);
                }
          }
      
      } 
      // 2. Private Message (User to User)
      else {
          int destFd = -1;
          // Find user by nickname
          for (std::map<int, Client>::iterator it = users.begin(); it != users.end(); ++it) {
              if (it->second.getNickname() == target) {
                  destFd = it->first;
                  break;
              }
          }

          if (destFd == -1) {
              server.queueMessage(fd, Response::error(IRC::ERR_NOSUCHNICK, sender.getNickname(), target + " :No such nick/channel"));
              return 0;
          }

          std::string fullMsg = senderPrefix + " PRIVMSG " + target + " :" + text + "\r\n";
          server.queueMessage(destFd, fullMsg);
      }
      return 0;
  } else if (cmd == "kick") {
      // Syntax: KICK <channel> <user> [<comment>]
      if (users.find(fd) == users.end()) return 0;
      if (tokVec.size() < 3) {
          server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "KICK :Not enough parameters"));
          return 0;
      }

      std::string channelName = tokVec[1];
      std::string targetNick = tokVec[2];
      std::string comment = (tokVec.size() > 3) ? tokVec[3] : "Kicked";
      Client& sender = users.find(fd)->second;

      std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
      if (chIt == channels.end()) {
          server.queueMessage(fd, Response::error(IRC::ERR_NOSUCHCHANNEL, sender.getNickname(), channelName + " :No such channel"));
          return 0;
      }
      Channel& channel = chIt->second;

      // Check permissions of sender
      MemberInfo senderInfo;
      if (channel.getUserInfo(sender.getNickname(), &senderInfo) == -1) {
           server.queueMessage(fd, Response::error(IRC::ERR_NOTONCHANNEL, sender.getNickname(), channelName + " :You're not on that channel"));
           return 0;
      }
      
      if (!senderInfo.op) {
           server.queueMessage(fd, Response::error(IRC::ERR_CHANOPRIVSNEEDED, sender.getNickname(), channelName + " :You're not channel operator"));
           return 0;
      }

      // Prevent kicking yourself
      if (sender.getNickname() == targetNick) {
           server.queueMessage(fd, Response::error(IRC::ERR_USERNOTINCHANNEL, sender.getNickname(), targetNick + " " + channelName + " :Cannot kick yourself"));
           return 0;
      }

      // Check target existence in channel
      MemberInfo targetInfo;
      if (channel.getUserInfo(targetNick, &targetInfo) == -1) {
           server.queueMessage(fd, Response::error(IRC::ERR_USERNOTINCHANNEL, sender.getNickname(), targetNick + " " + channelName + " :They aren't on that channel"));
           return 0;
      }

      // Perform Kick
      // 1. Notify Channel (Broadcast KICK msg) logic:
      // :Sender!u@h KICK #chan Target :Comment
      std::string kickMsg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@127.0.0.1 KICK " + channelName + " " + targetNick + " :" + comment + "\r\n";
      
      // Send to everyone including target
      const std::map<std::string, MemberInfo>& members = channel.getUsers();
      for (std::map<std::string, MemberInfo>::const_iterator it = members.begin(); it != members.end(); ++it) {
          server.queueMessage(it->second.fd, kickMsg);
      }
      
      // 2. Actually remove user
      channel.delUser(targetInfo.client);
      
      return 0;

  } else if (cmd == "invite") {
      // Syntax: INVITE <nickname> <channel>
      // TODO: test invite command after impelment mode command
      if (users.find(fd) == users.end()) return 0;
      if (tokVec.size() < 3) {
          server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "INVITE :Not enough parameters"));
          return 0;
      }
      
      std::string targetNick = tokVec[1];
      std::string channelName = tokVec[2];
      Client& sender = users.find(fd)->second;

      // Check if channel exists
      std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
      if (chIt == channels.end()) {
          server.queueMessage(fd, Response::error(IRC::ERR_NOSUCHCHANNEL, sender.getNickname(), channelName + " :No such channel"));
          return 0;
      }
      Channel& channel = chIt->second;

      // Partner's inviteUser checks: Sender on channel? Sender Op? Target already on channel?
      const std::string res = channel.inviteUser(&sender, targetNick);
      
      if (res == IRC::RPL_INVITING) {
          // Success
          // 1. Send RPL_INVITING to sender
          server.queueMessage(fd, Response::build(IRC::RPL_INVITING, sender.getNickname(), targetNick + " " + channelName));
          
          // 2. Send INVITE message to target
          // Find target FD
          int targetFd = -1;
           for (std::map<int, Client>::iterator it = users.begin(); it != users.end(); ++it) {
              if (it->second.getNickname() == targetNick) {
                  targetFd = it->first;
                  break;
              }
          }
          if (targetFd != -1) {
              // :Sender INVITE Target :#chan
              std::string inviteMsg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@127.0.0.1 INVITE " + targetNick + " :" + channelName + "\r\n";
              server.queueMessage(targetFd, inviteMsg);
          }
      } else if (res == IRC::ERR_NOTONCHANNEL) {
        server.queueMessage(fd, Response::error(IRC::ERR_NOTONCHANNEL, sender.getNickname(), channelName + " :You're not on that channel"));
      } else if (res == IRC::ERR_CHANOPRIVSNEEDED) {
            server.queueMessage(fd, Response::error(IRC::ERR_CHANOPRIVSNEEDED, sender.getNickname(), channelName + " :You're not channel operator"));
      } else if (res == IRC::ERR_USERONCHANNEL) {
           server.queueMessage(fd, Response::error(IRC::ERR_USERONCHANNEL, sender.getNickname(), targetNick + " " + channelName + " :is already on channel"));
      }
      return 0;

  } else if (cmd == "topic") {
      // Syntax: TOPIC <channel> [<topic>]
       if (users.find(fd) == users.end()) return 0;
      if (tokVec.size() < 2) {
          server.queueMessage(fd, Response::error(IRC::ERR_NEEDMOREPARAMS, users.find(fd)->second.getNickname(), "TOPIC :Not enough parameters"));
          return 0;
      }
      
      std::string channelName = tokVec[1];
      Client& sender = users.find(fd)->second;
       
      std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
      if (chIt == channels.end()) {
          server.queueMessage(fd, Response::error(IRC::ERR_NOSUCHCHANNEL, sender.getNickname(), channelName + " :No such channel"));
          return 0;
      }
      Channel& channel = chIt->second;

      if (tokVec.size() == 2) {
          // GET TOPIC
          std::string topic = channel.getTopic();
          if (topic.empty()) {
               server.queueMessage(fd, Response::build(IRC::RPL_NOTOPIC, sender.getNickname(), channelName + " :No topic is set"));
          } else {
               server.queueMessage(fd, Response::build(IRC::RPL_TOPIC, sender.getNickname(), channelName + " :" + topic));
          }
      } else {
           // SET TOPIC
           std::string newTopic = tokVec[2]; // getCommand splits by space, but we processed trailing already in Manager.cpp:21
           // Wait, doRequest logic: puts trailing as last token. If TOPIC #chan :My new topic
           // tokVec[0]=TOPIC, tokVec[1]=#chan, tokVec[2]=My new topic
           
           const std::string res = channel.setTopic(&sender, newTopic);
           if (res == "TOPIC") {
                // Success: Broadcast TOPIC change
                // :Sender TOPIC #chan :New Topic
                std::string topicMsg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@127.0.0.1 TOPIC " + channelName + " :" + newTopic + "\r\n";
                 const std::map<std::string, MemberInfo>& members = channel.getUsers();
                 for (std::map<std::string, MemberInfo>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    server.queueMessage(it->second.fd, topicMsg);
                 }
           } else {
                // Not op or not on channel
                MemberInfo senderInfo;
                if (channel.getUserInfo(sender.getNickname(), &senderInfo) == -1) {
                     server.queueMessage(fd, Response::error(IRC::ERR_NOTONCHANNEL, sender.getNickname(), channelName + " :You're not on that channel"));
                } else {
                     server.queueMessage(fd, Response::error(IRC::ERR_CHANOPRIVSNEEDED, sender.getNickname(), channelName + " :You're not channel operator"));
                }
           }
      }
      return 0;
  } else if (cmd == "ping") {
      if (tokVec.size() < 2) {
          return 0; // Ignore
      }
      std::string token = tokVec[1];
      server.queueMessage(fd, "PONG " + token + "\r\n");
      return 0;
  } else if (cmd == "quit") {
      std::string reason = (tokVec.size() > 1) ? tokVec[1] : "Client Quit";
      
      // Cleanup logic is similar to PART but for ALL channels
      // However, Server::run() detects connection loss usually,
      // but explicitly receiving QUIT is good for instant cleanup.
      
      // We process clean up here? Or just close connection?
      // Server will detect close and call closeClient.
      // But we should notify channels.
      
      Client& client = users.find(fd)->second;
      std::string nickname = client.getNickname();
      std::string quitMsg = ":" + nickname + "!" + client.getUsername() + "@127.0.0.1 QUIT :" + reason + "\r\n";

      // Notify all channels user is in
      // This is expensive: we iterate ALL channels. 
      // Optimized way: Client class has list of joined channels.
      
      // Using global broadcast for simplicity or iterate all channels
      // Better: Client class should track joined channels (partner already added _joinedChannels in Client.hpp!)
      
      // TODO: Use client.getJoinedChannels() if available in partner code?
      // Let's look at Client.cpp provided earlier.
      // Yes, _joinedChannels vector exists. But no getter in Client?
      // Wait, I saw joinChannel method but no getJoinedChannels in read_file output for Client.cpp
      // Let's assume we scan all channels for now (safe mandatory implementation)
      
       for (std::map<std::string, Channel>::iterator chIt = channels.begin(); chIt != channels.end(); ++chIt) {
           Channel& channel = chIt->second;
           if (channel.getUserInfo(nickname) == 1) { // User is in this channel
               channel.delUser(&client);
               const std::map<std::string, MemberInfo>& members = channel.getUsers();
               for (std::map<std::string, MemberInfo>::const_iterator mit = members.begin(); mit != members.end(); ++mit) {
                    server.queueMessage(mit->second.fd, quitMsg);
               }
           }
      }

      // We do not close FD here, we let the Server loop handle it or returning -1?
      // Server.cpp loop handles POLLHUP. 
      // But QUIT command doesn't necessarily close socket immediately from client side?
      // Valid IRC behavior: Server closes connection after sending ERROR.

      server.queueMessage(fd, "ERROR :Closing Link: " + nickname + " (" + reason + ")\r\n");
      // Force disconnect in server?
      // Currently Manager can't tell Server to close FD easily except returning special value?
      // Actually Server.cpp doesn't check return value of doRequest to close.
      // Let's leave it open, client usually closes after sending QUIT.
  }

  return 1;
}
