#ifndef PREZ_PARSERS_GENERATOR_BUILD_PARSER_HPP
#define PREZ_PARSERS_GENERATOR_BUILD_PARSER_HPP

#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/rules.hpp"
#include "src/parsers/generator/utils.hpp"

#include <optional>
#include <ostream>


DFA<DFARuleSet, int, DFARuleSetHash>
buildParserDFA(const GrammarData& gd, const GenerateFlags& generateFlags);
void condensedDFAToCode(
    std::ostream& out, const GrammarData& gd, const GenerateFlags& generateFlags);

#endif
