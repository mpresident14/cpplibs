#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/rules.hpp"
#include "src/parsers/generator/utils.hpp"

#include <optional>
#include <ostream>


DFA<DFARuleSet, int, DFARuleSetHash> buildParserDFA(
    const GrammarData& gd, const ParseFlags& parseFlags);
void condensedDFAToCode(std::ostream& out, const GrammarData& gd, const ParseFlags& parseFlags);

#endif
