#include "src/parsers/generator/regex_merge.hpp"

#include "src/logger/logger.hpp"
#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/regex.hpp"
#include "src/parsers/generator/regex_parser.hpp"

#include <cstddef>
#include <functional>
#include <numeric>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <prez/print_stuff.hpp>

using namespace std;

/*************
 * Regex DFA *
 *************/
namespace {

constexpr char alphabet[] =
    " !\"#$%&\'()*+,-./"
    "\\0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`"
    "abcdefghijklmnopqrstuvwxyz{|}~\n\t";

using RgxDFA = DFA<RgxPtr, char, Regex::PtrHash>;

/* BFS with regex derivative expansion */
RgxDFA buildRegexDFA(const RgxPtr& rgx) {
  RgxDFA dfa(rgx);
  queue<RgxDFA::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    RgxDFA::Node* node = q.front();
    RgxPtr rgx = node->getValue();
    q.pop();

    char c;
    for (size_t i = 0; (c = alphabet[i]) != '\0'; ++i) {
      RgxDFA::Node* successor = dfa.addTransition(node, c, rgx->getDeriv(c));
      if (successor) {
        q.push(successor);
      }
    }
  }

  return dfa;
}

/********************
 * Merged Regex DFA *
 *******************/

struct MergeData {
  vector<pair<RgxDFA::Node*, int>> states;
  int token;

  struct Hash {
    size_t operator()(const MergeData& mergeData) const noexcept {
      const vector<pair<RgxDFA::Node*, int>>& states = mergeData.states;
      return (size_t)accumulate(
          states.cbegin(), states.cend(), 0, [](uintptr_t n, pair<const RgxDFA::Node*, int> node) {
            return n ^ (uintptr_t)node.first;
          });
    }
  };
};
}  // namespace

namespace {

inline bool operator==(const MergeData& md1, const MergeData& md2) noexcept {
  return md1.states == md2.states;
}

using MergedRgxDFA = DFA<MergeData, char, MergeData::Hash>;

MergedRgxDFA buildMergedRgxDFA(const GrammarData& gd) {
  // Store the errors so we can throw them all at once.
  Logger logger;
  // We need the regex DFAs to be live long enough to create the merged DFA,
  // so we store them in this vector. Afterwards, all the pointers inside the
  // merged DFA will be deleted, but we will no longer need them.
  size_t numTokens = gd.tokens.size();
  vector<RgxDFA> rgxDfas;
  rgxDfas.reserve(numTokens);

  // Initialize the root of the merged DFA by creating DFAs for each regex
  // and mapping their roots to the appropriate token value
  vector<pair<RgxDFA::Node*, int>> initialStates;
  int stateToken = NONE;
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = gd.tokens[i];
    try {
      RgxPtr rgx = RgxPtr(regex_parser::parseString(token.regex));
      RgxDFA rgxDfa = buildRegexDFA(move(rgx));
      initialStates.push_back({ rgxDfa.getRoot(), arrIndToTok(i) });
      if (rgxDfa.getRoot()->getValue()->isNullable()) {
        // Accepting the empty string will likely result in an infinite loop
        logger.logWarning(
            token.declLine,
            string("The regex \"").append(token.regex).append("\" accepts the empty string"));
        // Multiple regex DFAs accept the empty string. We pick the regex that
        // was listed first.
        if (stateToken == NONE) {
          stateToken = arrIndToTok(i);
        }
      }
      rgxDfas.push_back(move(rgxDfa));
    } catch (const regex_parser::ParseException& e) {
      string_view err(e.what());
      logger.logError(
          token.declLine,
          string("For regex \"")
              .append(token.regex)
              .append("\":\n")
              .append(err.substr(err.find_first_of('\n') + 1)));
    }
  }

  if (logger.hasErrors()) {
    ostringstream errors;
    logger.streamLog(errors);
    throw regex_parser::ParseException(errors.str());
  }

  MergedRgxDFA mergedDfa(MergeData{ move(initialStates), stateToken });

  // BFS to build the merged DFA
  queue<MergedRgxDFA::Node*> q;
  q.push(mergedDfa.getRoot());

  while (!q.empty()) {
    MergedRgxDFA::Node* mergedNode = q.front();
    q.pop();
    char c;
    for (size_t i = 0; (c = alphabet[i]) != '\0'; ++i) {
      vector<pair<RgxDFA::Node*, int>> newStates;
      stateToken = NONE;
      for (auto& nodeAndToken : mergedNode->getValue().states) {
        RgxDFA::Node* node = nodeAndToken.first;
        int token = nodeAndToken.second;
        // Since Regex DFAs are actual DFAs (they have a transition for every
        // symbol in the alphabet), each node always has a successor for every
        // transition.
        RgxDFA::Node* successor = RgxDFA::step(node, c);
        newStates.push_back({ successor, token });
        if (successor->getValue()->isNullable()) {
          // Multiple regex DFAs accept the same string. We pick the regex
          // that was listed first (tokens are negative, so greater tokens listed first)
          if (token > stateToken) {
            stateToken = token;
          }
        }
      }
      // Add transition to the merged node and add it to the queue if it did
      // not already exist in the merged DFA. Again, since Regex DFAs are
      // actual DFAs, we are guaranteed to have a valid state in newStates for
      // each Regex DFA.
      MergedRgxDFA::Node* mergedSuccessor =
          mergedDfa.addTransition(mergedNode, c, { newStates, stateToken });
      if (mergedSuccessor) {
        q.push(mergedSuccessor);
      }
    }
  }

  return mergedDfa;
}

/**********************************
 * Write Merged Regex DFA to file *
 **********************************/

/* Value string representation */
string charToString(char c) {
  switch (c) {
    case '\\':
      return { '\'', '\\', '\\', '\'' };
    case '\'':
      return { '\'', '\\', '\'', '\'' };
    case '\n':
      return { '\'', '\\', 'n', '\'' };
    case '\t':
      return { '\'', '\\', 't', '\'' };
    default:
      return string{ '\'', c, '\'' };
  }
}

}  // namespace

void mergedRgxDFAToCode(ostream& out, const GrammarData& gd) {
  buildMergedRgxDFA(gd).streamAsCode(
      out,
      "int",
      "char",
      [](const MergeData& mergedData) { return to_string(mergedData.token); },
      charToString);
}
