#include "src/parsers/generator/regex_parse.hpp"

#include "src/parsers/generator/build_parser.hpp"
#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/rules.hpp"
#include "src/parsers/generator/utils.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

using namespace std;

/***********
 * GRAMMAR *
 ***********/

/* CHAR { char :: #str[0] } <DELETER OPTIONAL>
 *
 *
 * Regex { Regex* } { delete #obj; }
 * Regex := Alt                              { #0 }
 *        | Concats                          { new Concat(move(#0)) }
 *        | Regex STAR                       { new Star(#0) }
 *        | Not                              { #0 }
 *        | LBRACKET CHAR DASH CHAR RBRACKET { new Range(#1, #3) }
 *        | LPAREN Regex RPAREN              { #1 }
 *        | CHAR                             { new Character(#0) }
 *        | LBRACKET CHAR RBRACKET           { new Character(#1))) }
 *        | DOT                              { new Dot() }
 *
 * Alt { Regex* } { delete #obj; }
 * Alt := Alts                       { new Alt(move(#0)) }
 *      | LBRACKET Concats RBRACKET  { new Alt(move(#1)) }
 *
 * Not { Regex* } { delete #obj; }
 * Not := LBRACKET CARET CHAR RBRACKET            { new Not(new Character(#2)))
 * } | LBRACKET CARET Concats RBRACKET         { new Not(new Alt(move(#2))) } |
 * LBRACKET CARET CHAR DASH CHAR RBRACKET  { new Not(new Range(#2, #4)) }
 *
 * Alts { RegexVector }
 * Alts := Regex BAR Regex { RegexVector(#0, #2) }
 *       | Regex BAR Alts  { RegexVector(#0, move(#2)) }
 *
 * Concats { RegexVector* }
 * Concats := Regex Regex   { RegexVector(#0, #1) }
 *          | Regex Concats { RegexVector(#0, move(#1)) }
 *
 *
 * */

namespace {

/* Tokens */
enum TokenEnum {
  BAR = -1,
  STAR = -2,
  CARET = -3,
  LBRACKET = -4,
  RBRACKET = -5,
  LPAREN = -6,
  RPAREN = -7,
  DASH = -8,
  CHAR = -9,
  DOT = -10,
};

/* Variables */
enum VarEnum {
  REGEX = 1,
  ALT,
  NOT,
  ALTS,
  CONCATS,
};

/* Concrete Types */
enum ConcreteEnum {
  REGEX_ALT = 1,
  REGEX_CONCATS,
  REGEX_STAR,
  REGEX_NOT,
  REGEX_RANGE,
  REGEX_GROUP,
  REGEX_CHAR,
  REGEX_BRACKET_CHAR,
  REGEX_DOT,
  ALT_ALTS,
  ALT_BRACKET,
  NOT_CHAR,
  NOT_CONCATS,
  NOT_RANGE,
  ALTS_REGEX,
  ALTS_ALTS,
  CONCATS_REGEX,
  CONCATS_CONCATS,
};

// NOTE: These must be in same order as the constants above.
struct GrammarData GRAMMAR_DATA = {
  /* tokens */ {
      { "BAR", "", 1, Assoc::LEFT, "", "", "" },
      { "STAR", "", 6, Assoc::LEFT, "", "", "" },
      { "CARET", "", NONE, Assoc::RIGHT, "", "", "" },
      { "LBRACKET", "", 3, Assoc::NONE, "", "", "" },
      { "RBRACKET", "", 5, Assoc::NONE, "", "", "" },
      { "LPAREN", "", 3, Assoc::NONE, "", "", "" },
      { "RPAREN", "", 3, Assoc::NONE, "", "", "" },
      { "DASH", "", NONE, Assoc::NONE, "", "", "" },
      { "CHAR", "", 3, Assoc::LEFT, "", "", "" },
      { "DOT", "", 3, Assoc::LEFT, "", "", "" },
  },

  /* concretes */
  { { "SCONC", S, NONE, { REGEX }, "" },
    { "REGEX_ALT", REGEX, NONE, { ALT }, "" },
    { "REGEX_CONCATS", REGEX, NONE, { CONCATS }, "" },
    { "REGEX_STAR", REGEX, NONE, { REGEX, STAR }, "" },
    { "REGEX_NOT", REGEX, NONE, { NOT }, "" },
    { "REGEX_RANGE", REGEX, NONE, { LBRACKET, CHAR, DASH, CHAR, RBRACKET }, "" },
    { "REGEX_GROUP", REGEX, NONE, { LPAREN, REGEX, RPAREN }, "" },
    { "REGEX_CHAR", REGEX, NONE, { CHAR }, "" },
    { "REGEX_BRACKET_CHAR", REGEX, NONE, { LBRACKET, CHAR, RBRACKET }, "" },
    { "REGEX_DOT", REGEX, NONE, { DOT }, "" },
    { "ALT_ALTS", ALT, NONE, { ALTS }, "" },
    { "ALT_BRACKET", ALT, NONE, { LBRACKET, CONCATS, RBRACKET }, "" },
    { "NOT_CHAR", NOT, NONE, { LBRACKET, CARET, CHAR, RBRACKET }, "" },
    { "NOT_CONCATS", NOT, NONE, { LBRACKET, CARET, CONCATS, RBRACKET }, "" },
    { "NOT_RANGE", NOT, NONE, { LBRACKET, CARET, CHAR, DASH, CHAR, RBRACKET }, "" },
    { "ALTS_REGEX", ALTS, NONE, { REGEX, BAR, REGEX }, "" },
    { "ALTS_ALTS", ALTS, NONE, { ALTS, BAR, REGEX }, "" },
    { "CONCATS_REGEX", CONCATS, 4, { REGEX, REGEX }, "" },
    { "CONCATS_CONCATS", CONCATS, 4, { CONCATS, REGEX }, "" } },

  /* variables */
  { { "S", "", { SCONC }, "" },
    { "Regex",
      "",
      { REGEX_ALT,
        REGEX_CONCATS,
        REGEX_STAR,
        REGEX_NOT,
        REGEX_RANGE,
        REGEX_GROUP,
        REGEX_CHAR,
        REGEX_BRACKET_CHAR,
        REGEX_DOT },
      "" },
    { "Alt",
      "",
      {
          ALT_ALTS,
          ALT_BRACKET,
      },
      "" },
    { "Not",
      "",
      {
          NOT_CHAR,
          NOT_CONCATS,
          NOT_RANGE,
      },
      "" },
    { "Alts",
      "",
      {
          ALTS_REGEX,
          ALTS_ALTS,
      },
      "" },
    { "Concats",
      "",
      {
          CONCATS_REGEX,
          CONCATS_CONCATS,
      },
      "" } }
};

/*************
 * STACK_OBJ *
 *************/

struct Start {
  Start(Regex* r) : r_(r) {}
  Regex* r_;
};

class StackObj {
public:
  StackObj(int symbol, void* obj) : symbol_(symbol), obj_(obj) {}
  StackObj(StackObj&& other)
      : symbol_(other.symbol_), obj_(other.obj_), released_(other.released_) {
    other.obj_ = nullptr;
  }
  StackObj(const StackObj& other) = delete;
  StackObj& operator=(const StackObj& other) = delete;
  StackObj& operator=(StackObj&& other) = delete;

  // A StackObj is always responsible for deleting its own pointer (unless
  // moved). However, once a StackObj releases its object, it is no longer
  // responsible for calling the custom deleter for that object (except in the
  // case of a parseError).
  ~StackObj() {
    if (!obj_) {
      return;
    }

    switch (symbol_) {
      case S:
        delete static_cast<Start*>(obj_);
        break;
      case REGEX:  // Fall thru
      case ALT:    // Fall thru
      case NOT:
        if (!released_) {
          delete *static_cast<Regex**>(obj_);
        }
        delete static_cast<Regex**>(obj_);
        break;
      case ALTS:
      case CONCATS:
        delete static_cast<RegexVector*>(obj_);
        break;
      case CHAR:
        delete static_cast<char*>(obj_);
        break;
      default:
        return;
    }
  }

  void* releaseObj() noexcept {
    released_ = true;
    return obj_;
  }
  int getSymbol() const noexcept { return symbol_; }
  void* getObj() const noexcept { return obj_; }
  void unrelease() noexcept { released_ = false; }

private:
  const int symbol_;
  void* obj_;
  bool released_ = false;
};

/*********
 * LEXER *
 *********/

void handleEscape(vector<StackObj>& tokens, char c) {
  switch (c) {
    case 'n':
      tokens.emplace_back(CHAR, new char('\n'));
      break;
    case 't':
      tokens.emplace_back(CHAR, new char('\t'));
      break;
    default:
      tokens.emplace_back(CHAR, new char(c));
  }
}

vector<StackObj> lex(const string& input) {
  vector<StackObj> tokens;
  bool escaped = false;
  size_t leftBracket = 0;
  bool caret = false;
  tokens.reserve(input.size());

  for (char c : input) {
    // All characters within brackets are just literals except '-' for a range
    // and the first '^'
    if (leftBracket > 0) {
      if (escaped) {
        handleEscape(tokens, c);
        escaped = false;
        ++leftBracket;
        continue;
      }

      if (c == '^' && leftBracket == 1) {
        tokens.emplace_back(CARET, nullptr);
        caret = true;
        leftBracket = 2;
      } else if (c == '-' && ((leftBracket == 2 && !caret) || (leftBracket == 3 && caret))) {
        tokens.emplace_back(DASH, nullptr);
        leftBracket = 4;
      } else if (c == '\\') {
        escaped = true;
      } else if (c == ']') {
        tokens.emplace_back(RBRACKET, nullptr);
        leftBracket = 0;
        caret = false;
      } else {
        tokens.emplace_back(CHAR, new char(c));
        ++leftBracket;
      }
      continue;
    }

    if (escaped) {
      handleEscape(tokens, c);
      escaped = false;
      continue;
    }

    switch (c) {
      case '.':
        tokens.emplace_back(DOT, nullptr);
        break;
      case '|':
        tokens.emplace_back(BAR, nullptr);
        break;
      case '*':
        tokens.emplace_back(STAR, nullptr);
        break;
      case '[':
        tokens.emplace_back(LBRACKET, nullptr);
        leftBracket = 1;
        break;
      case '(':
        tokens.emplace_back(LPAREN, nullptr);
        break;
      case ')':
        tokens.emplace_back(RPAREN, nullptr);
        break;
      case '\\':
        escaped = true;
        break;
      default:
        tokens.emplace_back(CHAR, new char(c));
    }
  }

  return tokens;
}

/**********
 * PARSER *
 **********/

using CondensedDFA = DFA<RuleData, int>;
using CondensedNode = CondensedDFA::Node;

const CondensedDFA PARSER_DFA =
    buildParserDFA(GRAMMAR_DATA).convert<RuleData>([](const DFARuleSet& ruleSet) {
      return condenseRuleSet(ruleSet, GRAMMAR_DATA);
    });

/* Construct a concrete object */
void* constructObj(int concrete, StackObj* args) {
  switch (concrete) {
    case REGEX_ALT:
      return new Regex*(*static_cast<Regex**>(args[0].releaseObj()));
    case REGEX_CONCATS:
      return new Regex*(new Concat(move(*static_cast<RegexVector*>(args[0].releaseObj()))));
    case REGEX_STAR:
      return new Regex*(new Star(*static_cast<Regex**>(args[0].releaseObj())));
    case REGEX_NOT:
      return new Regex*(*static_cast<Regex**>(args[0].releaseObj()));
    case REGEX_RANGE:
      return new Regex*(new Range(
          *static_cast<char*>(args[1].releaseObj()), *static_cast<char*>(args[3].releaseObj())));
    case REGEX_GROUP:
      return new Regex*(*static_cast<Regex**>(args[1].releaseObj()));
    case REGEX_CHAR:
      return new Regex*(new Character(*static_cast<char*>(args[0].releaseObj())));
    case REGEX_BRACKET_CHAR:
      return new Regex*(new Character(*static_cast<char*>(args[1].releaseObj())));
    case REGEX_DOT:
      return new Regex*(new Dot());
    case ALT_ALTS:
      return new Regex*(new Alt(move(*static_cast<RegexVector*>(args[0].releaseObj()))));
    case ALT_BRACKET:
      return new Regex*(new Alt(move(*static_cast<RegexVector*>(args[1].releaseObj()))));
    case NOT_CHAR:
      return new Regex*(new Not(new Character(*static_cast<char*>(args[2].releaseObj()))));
    case NOT_CONCATS:
      return new Regex*(new Not(new Alt(move(*static_cast<RegexVector*>(args[2].releaseObj())))));
    case NOT_RANGE:
      return new Regex*(new Not(new Range(
          *static_cast<char*>(args[2].releaseObj()), *static_cast<char*>(args[4].releaseObj()))));
    case ALTS_REGEX:
      return new RegexVector(RegexVector(
          *static_cast<Regex**>(args[0].releaseObj()),
          *static_cast<Regex**>(args[2].releaseObj())));
    case ALTS_ALTS:
      return new RegexVector(RegexVector(
          move(*static_cast<RegexVector*>(args[0].releaseObj())),
          *static_cast<Regex**>(args[2].releaseObj())));
    case CONCATS_REGEX:
      return new RegexVector(RegexVector(
          *static_cast<Regex**>(args[0].releaseObj()),
          *static_cast<Regex**>(args[1].releaseObj())));
    case CONCATS_CONCATS:
      return new RegexVector(RegexVector(
          move(*static_cast<RegexVector*>(args[0].releaseObj())),
          *static_cast<Regex**>(args[1].releaseObj())));
    case SCONC:
      return new Start(move(*static_cast<Regex**>(args[0].releaseObj())));
    default:
      throw invalid_argument("Can't construct. Out of options.");
  }
}

StackObj construct(int concrete, StackObj* args) {
  return StackObj(GRAMMAR_DATA.concretes[concrete].varType, constructObj(concrete, args));
}

void parseError(vector<StackObj>& stk, const vector<StackObj>& inputTokens, size_t tokenPos) {
  // Need StackObjs to deep delete their objects
  for_each(stk.begin(), stk.end(), mem_fun_ref(&StackObj::unrelease));

  ostringstream errMsg;
  vector<string> stkSymbolNames;
  vector<string> remainingTokenNames;
  auto stkObjToName = [](const StackObj& stkObj) {
    if (isToken(stkObj.getSymbol())) {
      return GRAMMAR_DATA.tokens[tokenToFromIndex(stkObj.getSymbol())].name;
    }
    return GRAMMAR_DATA.variables[stkObj.getSymbol()].name;
  };

  transform(stk.begin(), stk.end(), back_inserter(stkSymbolNames), stkObjToName);
  transform(
      inputTokens.begin() + tokenPos,
      inputTokens.end(),
      back_inserter(remainingTokenNames),
      stkObjToName);

  errMsg << "No parse:\n\tStack: " << stkSymbolNames
         << "\n\tRemaining tokens: " << remainingTokenNames;
  throw invalid_argument(errMsg.str());
}

int tryReduce(
    const CondensedNode* node,
    int nextToken,
    vector<StackObj>& stk,
    const vector<StackObj>& inputTokens,
    size_t tokenPos) {
  const RuleData& ruleData = node->getValue();

  // No reducible rule, so try shifting
  if (!ruleData.reducibleRule.has_value()) {
    return NONE;
  }

  const DFARule& rule = *ruleData.reducibleRule;

  // Check if next token is in the lookahead set
  if (nextToken != NONE && !rule.lookahead[tokenToFromIndex(nextToken)]) {
    return NONE;
  }

  // Check if rule matches the stack
  // NOTE: stk is necessarily as large as the reducible rule, or we couldn't
  // have stepped through the DFA to get to this reducible rule
  if (!equal(
          rule.symbols.crbegin(),
          rule.symbols.crend(),
          stk.crbegin(),
          [](int symbol, const StackObj& stkObj) { return stkObj.getSymbol() == symbol; })) {
    return NONE;
  }

  // Can't shift, so reduce. After this point, nextToken
  // is guaranteed to not be NONE
  if (!node->getTransitions().contains(nextToken)) {
    return ruleData.reducibleRule->concrete;
  }

  // If both are options, then we resolve ambiguities in the following manner
  // (from https://www.haskell.org/happy/doc/html/sec-Precedences.html):
  // - If the precedence of the rule is higher, then the conflict is resolved
  // as a reduce.
  // - If the precedence of the lookahead token is higher, then the conflict
  // is resolved as a shift.
  // - If the precedences are equal, then
  //    ~ If the token is left-associative, then reduce
  //    ~ If the token is right-associative, then shift
  //    ~ If the token is non-associative, then fail
  // - If neither the rule nor the token have precedence, then the default is
  // to shift and a
  //   conflict will have been reported by findConflicts() in build_parser.cpp
  //   (note that this criterion differs slightly from Happy)

  const Token& nextTokenObj = GRAMMAR_DATA.tokens[tokenToFromIndex(nextToken)];
  int shiftPrecedence = nextTokenObj.precedence;

  // Unspecified precedence -> conflict! (Resolve by shifting)
  if (ruleData.precedence == NONE && shiftPrecedence == NONE) {
    return NONE;
  }

  if (ruleData.precedence > shiftPrecedence) {
    return ruleData.reducibleRule->concrete;
  }
  if (ruleData.precedence == shiftPrecedence) {
    if (nextTokenObj.assoc == Assoc::LEFT) {
      return ruleData.reducibleRule->concrete;
    }
    if (nextTokenObj.assoc == Assoc::NOT) {
      parseError(stk, inputTokens, tokenPos);
    }
  }

  // shift precedence is higher, or same and not left-associative, so shift
  return NONE;
}

/* Deep delete StackObj::obj starting at index i */
// void cleanPtrsFrom(const vector<StackObj>& stackObjs, size_t i) {
//   size_t size = stackObjs.size();
//   for (; i < size; ++i) {
//     deleteObj(stackObjs[i]);
//   }
// }

Regex* shiftReduce(vector<StackObj>& inputTokens) {
  vector<StackObj> stk;
  if (inputTokens.empty()) {
    parseError(stk, inputTokens, 0);
  }

  stk.push_back(move(inputTokens[0]));
  vector<CondensedNode*> dfaPath = { PARSER_DFA.getRoot() };
  size_t i = 1;
  size_t inputSize = inputTokens.size();

  // Stop when we have consumed all the input and the root of grammar
  // is the only thing on the stack
  while (!(i == inputSize && stk.size() == 1 && stk[0].getSymbol() == S)) {
    // Advance the DFA.
    CondensedNode* currentNode = PARSER_DFA.step(dfaPath.back(), stk.back().getSymbol());
    if (currentNode == nullptr) {
      // cleanPtrsFrom(stk, 0);
      // cleanPtrsFrom(inputTokens, i);
      parseError(stk, inputTokens, i);
    }
    dfaPath.push_back(currentNode);

    int nextInputToken = i == inputSize ? NONE : inputTokens[i].getSymbol();
    int concrete = tryReduce(currentNode, nextInputToken, stk, inputTokens, i);
    // Reduce
    if (concrete != NONE) {
      // Construct the new object, pop the arguments off the stack,
      // and push the new object onto it.
      size_t reduceStart = stk.size() - currentNode->getValue().reducibleRule->symbols.size();
      StackObj newObj = construct(concrete, &stk.data()[reduceStart]);
      size_t stkSize = stk.size();
      for (size_t j = 0; j < stkSize - reduceStart; ++j) {
        // deleteObjPtr(stk.back());
        stk.pop_back();
        dfaPath.pop_back();
      }
      stk.push_back(move(newObj));
    } else {  // Shift
      // No more tokens, didn't reduce to S
      if (i == inputSize) {
        // cleanPtrsFrom(stk, 0);
        parseError(stk, inputTokens, i);
      }
      stk.push_back(move(inputTokens[i]));
      ++i;
    }
  }

  // Remove the actual grammar root from the fake root we encapsulated it with
  Start* start = static_cast<Start*>(stk[0].releaseObj());
  // Regex* root = start->r_;
  // delete start;
  return start->r_;
}
}  // namespace

RgxPtr parse(const string& input) {
  vector<StackObj> stackObjs = lex(input);
  return RgxPtr(shiftReduce(stackObjs));
}
