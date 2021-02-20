#include "src/parsers/generator/pgen_parse.hpp"

#include "src/logger/logger.hpp"
#include "src/parsers/generator/pgen_grammar.hpp"
#include "src/parsers/generator/pgen_lexer.hpp"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace pgen_lexer;

namespace {
GrammarData gd;
vector<Token>& gdTokens = gd.tokens;
vector<Concrete>& gdConcretes = gd.concretes;
vector<Variable>& gdVariables = gd.variables;

unordered_map<string, size_t> tokenNameToIndex;
unordered_map<string, size_t> varNameToIndex;
unordered_map<string, int> precNameToPrec;
unordered_map<const string*, int> symbolToLineMap;

Logger logger;


class TokenStream {
public:
  TokenStream() = default;
  ~TokenStream() = default;
  TokenStream(const TokenStream& other) = delete;
  TokenStream& operator=(const TokenStream& other) = delete;
  TokenStream(TokenStream&& other) = delete;
  TokenStream& operator=(TokenStream&& other) = delete;

  void parseError(int tokenId) {
    logger.logFatal(
        tokens_[pos_].getLine(),
        string("Expected ")
            .append(symName(tokenId, CONFIG_GRAMMAR))
            .append(". Got ")
            .append(symName(tokens_[pos_].getSymbol(), CONFIG_GRAMMAR)));
  }

  bool maybeConsume(int tokenId) {
    if (tokens_.size() == pos_ || tokens_[pos_].getSymbol() != tokenId) {
      return false;
    }
    ++pos_;
    return true;
  }

  void consume(int tokenId) {
    if (!maybeConsume(tokenId)) {
      parseError(tokenId);
    }
  }

  string* maybeConsumeString(int tokenId) {
    if (tokens_.size() == pos_ || tokens_[pos_].getSymbol() != tokenId) {
      return nullptr;
    }
    return (string*)tokens_[pos_++].getObj();
  }

  string consumeString(int tokenId) {
    string* strPtr = maybeConsumeString(tokenId);
    if (!strPtr) {
      parseError(tokenId);
    }
    return *strPtr;
  }

  size_t currentLine() const noexcept { return pos_ == 0 ? 0 : tokens_[pos_ - 1].getLine(); }

  void setTokens(vector<StackObj>&& tokens) { tokens_ = move(tokens); }
  void checkIfDone() const {
    if (pos_ != tokens_.size()) {
      logger.logError(tokens_[pos_].getLine(), "Tokens still remaining.");
    }
  }

private:
  vector<StackObj> tokens_;
  size_t pos_ = 0;
};

TokenStream tokenStream;


string parseHeader() {
  tokenStream.consume(HEADER);
  return tokenStream.consumeString(CODE);
}

string parseSource() {
  tokenStream.consume(SOURCE);
  return tokenStream.consumeString(CODE);
}

bool maybeParseToken() {
  bool skip = false;
  // Check for an identifier to see if there are any more tokens
  string* name = tokenStream.maybeConsumeString(IDENT);
  if (!name) {
    if (tokenStream.maybeConsume(SKIP)) {
      skip = true;
    } else {
      return false;
    }
  }


  gdTokens.push_back(Token());
  Token& gdToken = gdTokens.back();
  gdToken.regex = tokenStream.consumeString(STRLIT);
  gdToken.declLine = tokenStream.currentLine();

  // We indicate to the lexer that the token should be skipped by setting the
  // precedence to SKIP_TOKEN (this is useful for things such as comments)
  if (skip) {
    gdToken.precedence = SKIP_TOKEN;
    return true;
  }

  if (tokenNameToIndex.contains(*name)) {
    logger.logError(tokenStream.currentLine(), "Duplicate token " + *name);
  }

  gdToken.name = *name;
  // Keep track of the index for this name so that we can use it when parsing
  // #prec
  tokenNameToIndex.emplace(move(*name), gdTokens.size() - 1);
  // An arrow signifies that the token holds data
  if (tokenStream.maybeConsume(ARROW)) {
    gdToken.type = tokenStream.consumeString(TYPE);
    gdToken.ctorExpr = tokenStream.consumeString(CODE);
    string* dtor = tokenStream.maybeConsumeString(CODE);
    if (dtor) {
      gdToken.dtorStmt = move(*dtor);
    }
  }
  return true;
}

void parseTokens() {
  tokenStream.consume(TOKENS);
  while (maybeParseToken())
    ;
}

bool maybeParsePrec(int prec) {
  vector<string*> precNames;
  // Check for an identifier to see if there are any more precedence lines
  string* precName;
  while ((precName = tokenStream.maybeConsumeString(IDENT))) {
    precNames.push_back(precName);
  }
  if (precNames.empty()) {
    return false;
  }

  // Figure out the associativity (which is required if precedence is
  // specified)
  Assoc assoc;
  if (tokenStream.maybeConsume(LEFTASSOC)) {
    assoc = Assoc::LEFT;
  } else if (tokenStream.maybeConsume(RIGHTASSOC)) {
    assoc = Assoc::RIGHT;
  } else {
    tokenStream.consume(NONASSOC);
    assoc = Assoc::NOT;
  }

  for (string* name : precNames) {
    // Look up the index of the token in the map
    auto iter = tokenNameToIndex.find(*name);
    if (iter == tokenNameToIndex.end()) {
      // This is not an actual token, just a placeholder to override a rule's
      // precedence
      precNameToPrec.emplace(move(*name), prec);
      continue;
    }

    Token& gdToken = gdTokens[iter->second];
    gdToken.precedence = prec;
    gdToken.assoc = assoc;
  }

  return true;
}

void parsePrecs() {
  tokenStream.consume(PREC);
  int prec = 1;
  while (maybeParsePrec(prec++))
    ;
}

bool maybeParseGrammarDecl() {
  // Check for an identifier to see if there are any more declarations
  string* name = tokenStream.maybeConsumeString(IDENT);
  if (!name) {
    return false;
  }

  gdVariables.push_back(Variable());
  Variable& gdVariable = gdVariables.back();
  gdVariable.name = *name;
  gdVariable.declLine = tokenStream.currentLine();
  varNameToIndex.emplace(move(*name), gdVariables.size() - 1);
  tokenStream.consume(ARROW);
  gdVariable.type = tokenStream.consumeString(TYPE);
  string* dtor = tokenStream.maybeConsumeString(CODE);
  if (dtor) {
    gdVariable.dtorStmt = *dtor;
  }
  return true;
}

/* This must be called right after parseGrammarDecl() */
void parseGrammarDef(size_t concNum) {
  Variable& gdVariable = gdVariables.back();
  gdConcretes.push_back(Concrete());
  Concrete& gdConcrete = gdConcretes.back();
  gdVariable.concreteTypes.push_back(gdConcretes.size() - 1);
  gdConcrete.name = gdVariable.name + to_string(concNum);
  gdConcrete.varType = gdVariables.size() - 1;
  gdConcrete.declLine = tokenStream.currentLine();
  // Store argSymbols as string*s for now until we have seen all the
  // variables. Then, we will convert them to correct integral values.
  // Also store the line number in a map for error checking
  string* argSymbol;
  while ((argSymbol = tokenStream.maybeConsumeString(IDENT))) {
    gdConcrete.argSymbols.push_back((intptr_t)argSymbol);
    symbolToLineMap.emplace(argSymbol, tokenStream.currentLine());
  }

  if (tokenStream.maybeConsume(PREC)) {
    string tokenName = tokenStream.consumeString(IDENT);
    int prec = NONE;
    // Look up the index of the token in the map
    auto iter = tokenNameToIndex.find(tokenName);
    if (iter == tokenNameToIndex.end()) {
      auto precIter = precNameToPrec.find(tokenName);
      if (precIter == precNameToPrec.end()) {
        logger.logError(tokenStream.currentLine(), "Unknown token " + tokenName);
      } else {
        prec = precIter->second;
      }
    } else {
      prec = gdTokens[iter->second].precedence;
    }

    if (prec == NONE) {
      logger.logWarning(
          tokenStream.currentLine(),
          string("Token ").append(tokenName).append(
              " is used to override a rule's precedence, but has no precedence "
              "set."));
    }
    gdConcrete.precedence = prec;
  }

  gdConcrete.ctorExpr = tokenStream.consumeString(CODE);
}

bool maybeParseGrammarVar() {
  if (!maybeParseGrammarDecl()) {
    return false;
  }

  tokenStream.consume(DEFINED);
  size_t concNum = 0;
  parseGrammarDef(concNum++);
  while (tokenStream.maybeConsume(BAR)) {
    parseGrammarDef(concNum++);
  }
  return true;
}

void parseGrammar() {
  gdVariables.push_back(Variable{"S", "Start", {SCONC}, "", 0});
  gdConcretes.push_back(Concrete{"SCONC", S, NONE, {1}, "Start(#0)", 0});
  tokenStream.consume(GRAMMAR);
  while (maybeParseGrammarVar())
    ;

  // Translate the string pointers to token/variable ids now that we
  // have parsed the whole file (skip SCONC)
  size_t numConcs = gdConcretes.size();
  for (size_t i = 1; i < numConcs; ++i) {
    Concrete& concrete = gdConcretes[i];
    size_t len = concrete.argSymbols.size();
    for (size_t j = 0; j < len; ++j) {
      const string& symbolName = *(string*)concrete.argSymbols[j];

      // Check if it is a token first
      auto tokIter = tokenNameToIndex.find(symbolName);
      if (tokIter == tokenNameToIndex.end()) {
        // Otherwise, check if it is a variable
        auto varIter = varNameToIndex.find(symbolName);
        if (varIter == varNameToIndex.end()) {
          logger.logError(symbolToLineMap.at(&symbolName), "Unknown symbol " + symbolName);
        } else {
          concrete.argSymbols[j] = varIter->second;
        }
      } else {
        concrete.argSymbols[j] = arrIndToTok(tokIter->second);
      }
    }
  }
}

} // namespace


GenerateInfo parseConfig(const string& filename, ostream& warnings) {
  ifstream configFile(filename);
  vector<StackObj> tokens = tokenize(configFile);
  if (tokens.empty()) {
    logger.logFatal(0, string("File ").append(filename).append(" is empty."));
  }

  tokenStream.setTokens(move(tokens));

  string addlHppCode = parseHeader();
  string addlCppCode = parseSource();
  parseTokens();
  parsePrecs();
  parseGrammar();
  tokenStream.checkIfDone();

  if (gdTokens.empty()) {
    logger.logError(0, "No tokens were provided.");
  }
  if (gdVariables.empty()) {
    logger.logError(0, "No grammar variables were provided.");
  }

  // Can't continue to parser generation if we had an error
  if (logger.hasErrors()) {
    throw Logger::Exception(logger);
  }
  // Record the warnings and continue
  logger.streamLog(warnings);

  return {move(gd), move(addlHppCode), move(addlCppCode)};
}
