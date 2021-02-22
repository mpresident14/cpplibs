#ifndef PREZ_PARSERS_GENERATOR_UTILS_HPP
#define PREZ_PARSERS_GENERATOR_UTILS_HPP

#include <climits>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>


enum class Assoc { NONE, LEFT, RIGHT, NOT };
inline std::ostream& operator<<(std::ostream& out, const Assoc& assoc) {
  switch (assoc) {
  case Assoc::LEFT:
    return out << "LEFT";
  case Assoc::RIGHT:
    return out << "RIGHT";
  case Assoc::NOT:
    return out << "NOT";
  default:
    return out << "NONE";
  }
}

static constexpr int NONE = INT_MIN;
static constexpr int SKIP_TOKEN = INT_MIN + 1;
static constexpr int EPSILON = 0;
static constexpr int S = 0;
static constexpr int SCONC = 0;

struct Token {
  std::string name;
  std::string type;
  int precedence = NONE;
  Assoc assoc = Assoc::NONE;
  std::string ctorExpr;
  std::string dtorStmt;
  std::string regex;
  size_t declLine;
};

struct Concrete {
  std::string name;
  int varType;
  int precedence = NONE;
  // See parseGrammarDef() in pgen_parse.cpp for why this is intptr_t
  std::vector<intptr_t> argSymbols;
  std::string ctorExpr;
  size_t declLine;
};

struct Variable {
  std::string name;
  std::string type;
  std::vector<int> concreteTypes;
  std::string dtorStmt;
  size_t declLine;
};

struct GrammarData {
  std::vector<Token> tokens;
  std::vector<Concrete> concretes;
  std::vector<Variable> variables;
};

constexpr bool isToken(int symbolId) noexcept { return symbolId < 0; }
/* Lookahead set has EPSILON as its 0th element */
constexpr int lookaheadInd(int tokenId) noexcept { return -tokenId; }
constexpr int tokToArrInd(int tokenId) noexcept { return -tokenId - 1; }
constexpr int arrIndToTok(size_t i) noexcept { return tokToArrInd((int)i); }

/* For use in array of both variables and tokens */
constexpr int symToArrInd(int symbolId, size_t numVars) noexcept {
  return isToken(symbolId) ? tokToArrInd(symbolId) + numVars : symbolId;
}
constexpr int arrIndToSym(size_t i, size_t numVars) noexcept {
  return i >= numVars ? numVars - i - 1 : i;
}

inline std::string symName(int symbolId, const GrammarData& gd) {
  if (isToken(symbolId)) {
    return gd.tokens[tokToArrInd(symbolId)].name;
  }
  return gd.variables[symbolId].name;
}

struct GenerateFlags {
  /*
   * Parser/Lexer will be written in <outDir>
   * parser/lexer cpp file will have '#include <includePath><name>.hpp'
   */
  std::string outDir;
  std::string includePathBase;
  std::string name;
  std::string logFile;
};

inline void intToCode(std::ostream& out, int n) { out << n; }

template <typename T, typename F>
void vecToCode(std::ostream& out, const std::vector<T>& v, F&& toCodeFn) {
  out << '{';
  for (const T& item : v) {
    toCodeFn(out, item);
    out << ',';
  }
  out << '}';
}

#endif
