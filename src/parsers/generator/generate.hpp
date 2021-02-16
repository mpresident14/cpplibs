#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "src/parsers/generator/parse_info.hpp"
#include "src/parsers/generator/utils.hpp"

#include <ostream>
#include <sstream>
#include <string_view>

void generateParserCode(
    const ParseInfo& parseInfo, const ParseFlags& parseFlags, std::ostream& warnings);

/*
 * Lexer will be written in <outputDir>
 * lexer.cpp will have '#include <lexerIncludePath><lexerName>.hpp'
 */
void generateLexerCode(
    const std::string& outputDir,
    const std::string& lexerIncludePath,
    const std::string& lexerName,
    const std::string& addlHdrIncludes,
    const GrammarData& gd);

#endif
