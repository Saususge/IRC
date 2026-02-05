#include "Validator.hpp"

#include <string>

namespace Validator {
bool isValidNickname(const std::string& nick) {
  if (nick.empty() || nick.length() > 9) {
    return false;
  }
  char first = nick[0];
  bool isFirstLetter =
      (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z');
  bool isFirstSpecial =
      std::string("[]\\`_^{|}").find(first) != std::string::npos;

  if (!isFirstLetter && !isFirstSpecial) {
    return false;
  }
  for (size_t i = 1; i < nick.length(); ++i) {
    char c = nick[i];
    bool isLetter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    bool isDigit = (c >= '0' && c <= '9');
    bool isSpecial = std::string("[]\\`_^{|}").find(c) != std::string::npos;
    bool isHyphen = (c == '-');
    if (!isLetter && !isDigit && !isSpecial && !isHyphen) {
      return false;
    }
  }
  return true;
}
}  // namespace Validator
