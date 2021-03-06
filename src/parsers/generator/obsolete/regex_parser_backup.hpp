#ifndef PREZ_PARSERS_GENERATOR_OBSOLETE_REGEX_PARSER_BACKUP_HPP
#define PREZ_PARSERS_GENERATOR_OBSOLETE_REGEX_PARSER_BACKUP_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include "src/parsers/generator/regex.hpp"

#include <istream>
#include <string>

#include <boost/dynamic_bitset.hpp>
namespace regex_parser {
Regex* parseString(const std::string& input);
Regex* parse(std::istream& input);
Regex* parse(const std::string& filename);
class ParseException : public std::exception {
public:
  ParseException(const std::string& errMsg) : errMsg_(errMsg) {}
  ParseException(const char* errMsg) : errMsg_(errMsg) {}
  const char* what() const noexcept override { return errMsg_.c_str(); }

private:
  std::string errMsg_;
};
} // namespace regex_parser
#endif
