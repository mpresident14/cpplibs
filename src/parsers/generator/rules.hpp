#ifndef PREZ_PARSERS_GENERATOR_RULES_HPP
#define PREZ_PARSERS_GENERATOR_RULES_HPP

#include "src/parsers/generator/utils.hpp"
#include "src/streams/stream.hpp"

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <unordered_set>
#include <vector>

#include <boost/dynamic_bitset.hpp>

struct DFARule {
public:
  constexpr bool atEnd() const noexcept { return pos == symbols.size(); }

  /* Given a rule "S -> A.B", returns B */
  constexpr int nextSymbol() const noexcept { return atEnd() ? NONE : symbols[pos]; }

  /* Given a rule "S -> A.BC", returns C */
  constexpr int nextNextSymbol() const noexcept {
    return pos >= symbols.size() - 1 ? NONE : symbols[pos + 1];
  }

  /* Given a rule "S -> A.B", returns "S -> AB." */
  DFARule nextStep() const noexcept { return {concrete, symbols, pos + 1, lookahead}; }

  // TODO: Use a member variable here
  int getPrecedence(const GrammarData& gd) const {
    // If no override precedence and the rule has a token, check precedence of
    // last token
    int overridePrec = gd.concretes[concrete].precedence;
    if (overridePrec == NONE) {
      // Find the last token, if any
      auto ruleIter = find_if(symbols.crbegin(), symbols.crend(), isToken);
      if (ruleIter != symbols.crend()) {
        return gd.tokens[tokToArrInd(*ruleIter)].precedence;
      } else {
        return NONE;
      }
    } else {
      return overridePrec;
    }
  }

  bool operator==(const DFARule& other) const noexcept {
    // DFARules are uniquely identified by their concrete, position, and
    // lookahead set.
    return concrete == other.concrete && pos == other.pos && lookahead == other.lookahead;
  }

  friend std::ostream& operator<<(std::ostream& out, const DFARule& rule) {
    out << rule.concrete << " -> ";
    size_t len = rule.symbols.size();
    for (size_t i = 0; i < len; ++i) {
      if (i == rule.pos) {
        out << '.';
      }
      out << rule.symbols[i] << ' ';
    }
    if (rule.pos == len) {
      out << '.';
    }
    out << " :: " << rule.lookahead;
    return out;
  }

  /* Does not use lookahead set. See comment below */
  struct Hash {
    size_t operator()(const DFARule& rule) const noexcept {
      std::hash<int> intHasher;
      return intHasher(rule.concrete) ^ (intHasher((int)rule.pos) << 1);
    }
  };

  /* Nodes of the DFA. Has to be a set, not a vector, because two groups of
   * rules should be equal if they contain the same rules (in any order).
   * Individual rules in a RuleSet are not compared using the lookahead set
   * since there should never be two rules differing only by their lookahead set
   * in the same RuleSet. However, operator== in the DFARule class still
   * compares lookahead sets because RuleSet equality compares the rules with
   * this operator, and two RuleSets can differ based on the lookahead sets of
   * the rules they contain.
   * */
  struct Eq {
    bool operator()(const DFARule& left, const DFARule& right) const noexcept {
      return left.concrete == right.concrete && left.symbols == right.symbols &&
             left.pos == right.pos;
    }
  };


  int concrete;
  std::vector<intptr_t> symbols;
  size_t pos;
  // Allows us to change lookahead while it is inside of a RuleSet, which is ok
  // b/c lookahead is not involved in the hash function or equality for RuleSets
  mutable boost::dynamic_bitset<> lookahead;
};

using DFARuleSet = std::unordered_set<DFARule, DFARule::Hash, DFARule::Eq>;

struct DFARuleSetHash {
  size_t operator()(const DFARuleSet& ruleSet) const noexcept {
    DFARule::Hash hasher;
    return prez::streams::streamFrom(ruleSet).map(hasher).sum();
  }
};

#endif
