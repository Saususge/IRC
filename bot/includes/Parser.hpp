#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

struct Message {
  std::string prefix;
  std::string command;
  std::vector<std::string> params;
  std::string trailing;
};

class Parser {
 private:
  Parser();
  Parser(const Parser& src);
  Parser& operator=(const Parser& src);
  ~Parser();

 public:
  static Message parse(const std::string& raw);
};

#endif
