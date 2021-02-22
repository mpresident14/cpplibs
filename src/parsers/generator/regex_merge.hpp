#ifndef PREZ_PARSERS_GENERATOR_REGEX_MERGE_HPP
#define PREZ_PARSERS_GENERATOR_REGEX_MERGE_HPP

#include "src/parsers/generator/utils.hpp"

#include <ostream>

void mergedRgxDFAToCode(std::ostream& out, const GrammarData& gd);

#endif
