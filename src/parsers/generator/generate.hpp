#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "src/parsers/generator/parse_info.hpp"
#include "src/parsers/generator/utils.hpp"

#include <ostream>
#include <sstream>
#include <string_view>

void generateParserCode(
    const GenerateInfo& generateInfo, const GenerateFlags& generateFlags, std::ostream& warnings);

void generateLexerCode(const GenerateInfo& generateInfo, const GenerateFlags& generateFlags);

#endif
