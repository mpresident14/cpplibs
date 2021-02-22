#ifndef PREZ_PARSERS_GENERATOR_NULL_FIRST_HPP
#define PREZ_PARSERS_GENERATOR_NULL_FIRST_HPP

#include "src/parsers/generator/utils.hpp"

#include <utility>
#include <vector>

#include <boost/dynamic_bitset.hpp>

boost::dynamic_bitset<> getNullabilities(const GrammarData& gd);
/*
 * Returns two results:
 * 1) A bitset representing whether each token can be null.
 * 2) A vector of bitsets representing the set of tokens that can appear at the start of a rule for
 *    each token.
 */
std::pair<boost::dynamic_bitset<>, std::vector<boost::dynamic_bitset<>>>
getNullsAndFirsts(const GrammarData& gd);

#endif
