#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <string>

namespace Validator {
bool isValidNickname(const std::string& nick);
bool isChannelNameValid(const std::string& channelName);
};  // namespace Validator

#endif