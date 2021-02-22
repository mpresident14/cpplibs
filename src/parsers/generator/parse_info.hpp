#ifndef PREZ_PARSERS_GENERATOR_PARSE_INFO_HPP
#define PREZ_PARSERS_GENERATOR_PARSE_INFO_HPP

#include "src/parsers/generator/utils.hpp"

#include <string>

struct GenerateInfo {
  GrammarData gd;
  std::string addlHppCode;
  std::string addlCppCode;
};

#endif // PARSE_INFO_HPP
