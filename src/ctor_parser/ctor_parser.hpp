#ifndef CTOR_PARSER_HPP
#define CTOR_PARSER_HPP

#include <string>
#include <string_view>
#include <vector>

namespace ctor_parser {

struct ClassId {
  // std::string id;
  std::string name;
};

struct Result {
  ClassId classType;
  std::vector<ClassId> argTypes;
  size_t line;
  std::string filename;
};

class CtorParser {
public:
  Result parseCtor(std::string_view ctor, size_t line, const char* filename);


private:
  template<typename CondFn>
  std::string parseUntil(CondFn cond);

  void parseChar(char c);
  void skipWhitespace();

  std::string ctor_;
  size_t pos_;
};

}  // namespace ctor_parser

#endif  // CTOR_PARSER_HPP
