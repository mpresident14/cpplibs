#ifndef PARSE_INFO_HPP
#define PARSE_INFO_HPP

#include "src/parsers/generator/utils.hpp"

#include <string>

struct ParseInfo {
  GrammarData gd;
  std::string addlHppCode;
  std::string addlCppCode;
};

#endif // PARSE_INFO_HPP
