#include "src/ctor_parser/ctor_parser.hpp"

#include <cctype>
#include <iostream>
#include <sstream>

using namespace std;

namespace ctor_parser {


Result CtorParser::parseCtor(std::string_view ctor, size_t line, const char* filename) {
  ctor_ = ctor;
  pos_ = 0;

  auto isLeftParen = [](char c) { return c == '('; };
  auto isCommaOrRightParen = [](char c) { return c == ')' || c == ','; };
  auto isWhitespace = [](char c) { return isspace(c); };

  ClassId classType = { parseUntil(isLeftParen) };
  ++pos_;
  skipWhitespace();
  if (ctor_[pos_] == ')') {
    return Result{ move(classType), {}, line, filename };
  }

  vector<ClassId> argTypes;
  while (true) {
    argTypes.push_back({ parseUntil(isWhitespace) });
    parseUntil(isCommaOrRightParen);
    if (ctor_[pos_] == ')') {
      break;
    }
    ++pos_;
  }
  return Result{ move(classType), move(argTypes), line, filename };
}


template <typename CondFn>
string CtorParser::parseUntil(CondFn cond) {
  skipWhitespace();
  size_t len = ctor_.size();
  string result;
  char c;
  while (pos_ < len && !cond((c = ctor_[pos_]))) {
    if (!isspace(c)) {
      result.push_back(c);
    }
    pos_++;
  }

  if (pos_ == len) {
    throw runtime_error("Did not meet condition.");
  }

  return result;
}

void CtorParser::skipWhitespace() {
  size_t len = ctor_.size();
  while (pos_ < len && isspace(ctor_[pos_])) {
    ++pos_;
  }
}

}  // namespace ctor_parser
