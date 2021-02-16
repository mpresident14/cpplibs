#include "src/parsers/generator/null_first.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

using BitsetVars = boost::dynamic_bitset<>;
using BitsetTokens = boost::dynamic_bitset<>;
using BitRef = typename boost::dynamic_bitset<>::reference;

namespace {
using namespace std;


/* Iterate to find the least fixed point */
void computeNullabilities(
    BitsetVars& nullabilities, const vector<vector<vector<BitRef>>>& equations) {
  bool changed = true;
  size_t numVars = nullabilities.size();
  while (changed) {
    changed = false;
    for (size_t i = 0; i < numVars; ++i) {
      // If the nullability for this symbol is already true, no need to
      // evaluate it again
      if (nullabilities[i]) {
        continue;
      }

      const vector<vector<BitRef>>& disjunctions = equations[i];
      // Nullable if any of the conjunctions evaluate to true
      bool newValue =
          any_of(disjunctions.cbegin(), disjunctions.cend(), [](const vector<BitRef>& conj) {
            return all_of(conj.cbegin(), conj.cend(), [](const BitRef& bitref) { return bitref; });
          });
      if (newValue) {
        nullabilities.set(i);
        changed = true;
      }
    }
  }
}

struct UnionEquation {
  UnionEquation(size_t numTokens) : tokenSet(numTokens) {}
  BitsetTokens tokenSet;
  vector<BitsetTokens*> bitSetTokRefs;
};

/* Iterate to find the least fixed point */
void computeFirsts(vector<BitsetTokens>& firsts, const vector<UnionEquation>& equations) {
  bool changed = true;
  while (changed) {
    changed = false;
    size_t numVariables = equations.size();
    for (size_t i = 0; i < numVariables; ++i) {
      const UnionEquation& unionEq = equations[i];
      // Take union of all the bitsets
      BitsetTokens newValue = unionEq.tokenSet;
      for (const BitsetTokens* bitsetRef : unionEq.bitSetTokRefs) {
        newValue |= *bitsetRef;
      }
      // Compare to previous value and see if anything has changed
      if (newValue != firsts[i]) {
        firsts[i] = std::move(newValue);
        changed = true;
      }
    }
  }
}
}  // namespace

/* For each symbol in the grammar, the equations for each rule on the rhs
 * are a disjunction of conjunctions, which we represent with a
 * vector<vector<BitVarRef> */
BitsetVars getNullabilities(const GrammarData& gd) {
  size_t numVars = gd.variables.size();
  BitsetVars nullabilities(numVars);
  vector<vector<vector<BitRef>>> equations(numVars);

  for (size_t var = 0; var < numVars; ++var) {
    for (int concreteType : gd.variables[var].concreteTypes) {
      // Epsilon (i.e. empty) is always nullable, so this symbol is nullable
      const Concrete& rule = gd.concretes[concreteType];
      if (rule.argSymbols.empty()) {
        nullabilities[var] = true;
        break;
      }
      // Tokens are never nullable, so stop considering this rule
      if (std::any_of(rule.argSymbols.cbegin(), rule.argSymbols.cend(), isToken)) {
        continue;
      }
      // Otherwise build the conjunction bitset
      vector<BitRef> conjunctions;
      for (int rhsSymbol : rule.argSymbols) {
        conjunctions.push_back(nullabilities[rhsSymbol]);
      }
      equations[var].push_back(std::move(conjunctions));
    }
  }

  computeNullabilities(nullabilities, equations);
  return nullabilities;
}


std::pair<BitsetVars, std::vector<BitsetTokens>> getNullsAndFirsts(const GrammarData& gd) {
  size_t numVars = gd.variables.size();
  // Include EPSILON
  size_t numTokens = gd.tokens.size() + 1;

  // Initialize firsts with empty bitset vectors
  vector<BitsetTokens> firsts;
  firsts.reserve(numVars);
  for (size_t i = 0; i < numVars; ++i) {
    firsts.push_back(BitsetTokens(numTokens));
  }

  // Initialize equations with empty bitset vectors
  vector<UnionEquation> equations;
  equations.reserve(numVars);
  for (size_t i = 0; i < numVars; ++i) {
    equations.push_back(UnionEquation(numTokens));
  }

  BitsetVars nullabilities = getNullabilities(gd);

  for (size_t var = 0; var < numVars; ++var) {
    UnionEquation& unionEq = equations[var];
    for (int concreteType : gd.variables[var].concreteTypes) {
      for (int rhsSymbol : gd.concretes[concreteType].argSymbols) {
        // Tokens are never nullable, so nothing beyond it can be first
        if (isToken(rhsSymbol)) {
          unionEq.tokenSet[lookaheadInd(rhsSymbol)] = true;
          break;
        }

        // Add first of this variable to the equation. If the variable is
        // nullable also add the next symbol, and so forth
        unionEq.bitSetTokRefs.push_back(&firsts[rhsSymbol]);
        if (!nullabilities[rhsSymbol]) {
          break;
        }
      }
    }
  }

  computeFirsts(firsts, equations);
  return { std::move(nullabilities), std::move(firsts) };
}
