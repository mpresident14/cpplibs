#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "src/generator/dfa.hpp"
#include "src/generator/rules.hpp"
#include "src/generator/utils.hpp"

#include <optional>
#include <ostream>


DFA<DFARuleSet, int, DFARuleSetHash> buildParserDFA(
    const GrammarData& gd, const GenerateFlags& generateFlags);
void condensedDFAToCode(std::ostream& out, const GrammarData& gd, const GenerateFlags& generateFlags);

#endif
