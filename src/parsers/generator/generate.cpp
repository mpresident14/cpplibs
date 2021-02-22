#include "src/parsers/generator/generate.hpp"

#include "src/logger/logger.hpp"
#include "src/misc/ostreamable.hpp"
#include "src/parsers/generator/build_parser.hpp"
#include "src/parsers/generator/regex_merge.hpp"
#include "src/parsers/generator/regex_parser.hpp"
#include "src/parsers/generator/utils.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;
namespace fs = std::filesystem;

namespace {

Logger logger;

/***********
 * TO CODE *
 ***********/

inline void strToCode(ostream& out, const string& str) { out << '"' << str << '"'; }

inline void tokToCode(ostream& out, const Token& token) {
  out << '{';
  strToCode(out, token.name);
  out << ',';
  strToCode(out, token.type);
  out << ',';
  intToCode(out, token.precedence);
  out << ',';
  out << "Assoc::" << token.assoc << '}';
  // Other fields not needed for shift-reducing
}

inline void concToCode(ostream& out, const Concrete& concrete) {
  out << '{';
  intToCode(out, concrete.varType);
  out << '}';
  // Other fields not needed for shift-reducing
}

inline void varToCode(ostream& out, const Variable& var) {
  out << '{';
  strToCode(out, var.name);
  out << ',';
  strToCode(out, var.type);
  out << '}';
  // Other fields not needed for shift-reducing
}

inline void gdToCode(ostream& out, const GrammarData& gd) {
  out << '{';
  vecToCode(out, gd.tokens, tokToCode);
  out << ',';
  vecToCode(out, gd.concretes, concToCode);
  out << ',';
  vecToCode(out, gd.variables, varToCode);
  out << '}';
}

/*********************
 * STRING OPERATIONS *
 *********************/
void replaceAll(ostream& out, string_view str, const string& from, const string& to) {
  size_t startPos = 0;
  size_t endPos = 0;
  while ((endPos = str.find(from, startPos)) != string::npos) {
    out << str.substr(startPos, endPos - startPos) << to;
    startPos = endPos + from.length();
  }
  out << str.substr(startPos);
}

string replaceAll(string_view str, char from, char to) {
  string s;
  s.reserve(str.size());

  for (char c : str) {
    if (c == from) {
      s.push_back(to);
    } else {
      s.push_back(c);
    }
  }
  return s;
}

void streamSymbolNames(ostream& out, const vector<intptr_t>& symbols, const GrammarData& gd) {
  vector<string> symbolNames;
  transform(symbols.begin(), symbols.end(), back_inserter(symbolNames), [&gd](int symbol) {
    return symName(symbol, gd);
  });
  out << prez::misc::OStreamable(symbolNames);
}

string convertArgNum(
    string_view argIndexStr, int argIndex, const Concrete& concrete, const GrammarData& gd) {
  const vector<intptr_t>& argSymbols = concrete.argSymbols;
  // These are user-provided numbers, so check the bounds
  if (argIndex < 0) {
    ostringstream& error = logger.logError(concrete.declLine);
    error << "Index " << argIndex << " is < 0 for rule ";
    streamSymbolNames(error, argSymbols, gd);
    error << '\n';
    return "";
  }
  if ((size_t)argIndex >= argSymbols.size()) {
    ostringstream& error = logger.logError(concrete.declLine);
    error << "Index " << argIndex << " is greater than the number of elements in rule ";
    streamSymbolNames(error, argSymbols, gd);
    error << '\n';
    return "";
  }

  int argSymbol = argSymbols[argIndex];
  string symbolType =
      isToken(argSymbol) ? gd.tokens[tokToArrInd(argSymbol)].type : gd.variables[argSymbol].type;
  // Make sure the symbol has data associated with it (only necessary for
  // tokens)
  if (symbolType.empty()) {
    ostringstream& error = logger.logError(concrete.declLine);
    error << "Token " << symName(argSymbol, gd)
          << " is passed as an argument, but has no data associated with it.\n";
    return "";
  }

  return string("(*static_cast<")
      .append(symbolType)
      .append("*>(args[")
      .append(argIndexStr)
      .append("].releaseObj()))");
}

void replacePounds(ostream& out, const Concrete& concrete, const GrammarData& gd) {
  size_t i = 0;
  string_view ctor = concrete.ctorExpr;
  size_t len = ctor.size();
  const char line[] = "line";

  while (i < len) {
    if (ctor[i] == '#') {
      ++i;
      string_view afterPound = ctor.substr(i);
      if (afterPound.starts_with(line)) {
        if (concrete.argSymbols.size() == 0) {
          logger.logError(concrete.declLine, "#line cannot be used with empty rule");
        }
        out << "args[0].getLine()";
        i += sizeof(line) - 1;
      } else {
        size_t numDigits;
        int argIndex = stoi(string(afterPound), &numDigits);
        i += numDigits;
        out << convertArgNum(afterPound.substr(0, numDigits), argIndex, concrete, gd);
      }
    } else {
      out << ctor[i++];
    }
  }
}

string extractNamespace(const fs::path& includePath) { return includePath.stem(); }

string extractHeaderGuard(const fs::path& includePath) {
  string headerGuard = replaceAll(string(includePath.parent_path() / includePath.stem()), '/', '_');
  transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);
  return headerGuard.append("_HPP");
}

/****************
 * GRAMMAR DATA *
 ****************/

void tokenDecl(ostream& out) {
  out << R"(struct Token {
      std::string name;
      std::string type;
      int precedence;
      Assoc assoc;
    };
    )";
}

void concreteDecl(ostream& out) {
  out << R"(struct Concrete {
      int varType;
    };
    )";
}

void variableDecl(ostream& out) {
  out << R"(struct Variable {
      std::string name;
      std::string type;
    };
    )";
}

void gdDecl(ostream& out, const GrammarData& gd) {
  out << R"(struct GrammarData {
      std::vector<Token> tokens;
      std::vector<Concrete> concretes;
      std::vector<Variable> variables;
    };
    )";
  out << "GrammarData GRAMMAR_DATA = ";
  gdToCode(out, gd);
  out << ';';
}

/****************
 * STACK OBJECT *
 ****************/

void stackObjDef(ostream& out, const GrammarData& gd) {
  out << R"(
      class StackObj {
      public:
        StackObj(int symbol, void* obj, size_t line)
          : symbol_(symbol), obj_(obj), line_(line) {}
        StackObj(StackObj&& other)
          : symbol_(other.symbol_), obj_(other.obj_), line_(other.line_), released_(other.released_) {
          other.obj_ = nullptr;
        }
        StackObj(const StackObj& other) = delete;
        StackObj& operator=(const StackObj& other) = delete;
        StackObj& operator=(StackObj&& other) = delete;
    )";

  // Destructor
  out << R"(
      ~StackObj() {
        if (!obj_) {
          return;
        }

        switch (symbol_) {
    )";
  size_t numTokens = gd.tokens.size();
  // Case statement for each token with data
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = gd.tokens[i];
    if (!token.type.empty()) {
      out << "case " << arrIndToTok(i) << ':';
      if (!token.dtorStmt.empty()) {
        out << "if (!released_) {";
        replaceAll(out, token.dtorStmt, "#obj", "(*static_cast<" + token.type + "*>(obj_))");
        out << '}';
      }
      out << "delete static_cast<" << token.type << "*>(obj_); break;";
    }
  }

  size_t numVars = gd.variables.size();
  // Case statement for each variable
  for (size_t i = 0; i < numVars; ++i) {
    const Variable& var = gd.variables[i];
    out << "case " << i << ':';
    if (!var.dtorStmt.empty()) {
      out << "if (!released_) {";
      replaceAll(out, var.dtorStmt, "#obj", "(*static_cast<" + var.type + "*>(obj_))");
      out << '}';
    }
    out << "delete static_cast<" << var.type << "*>(obj_); break;";
  }
  out << "default: return;}}";

  out << R"(
        void* releaseObj() noexcept {
          released_ = true;
          return obj_;
        }
        int getSymbol() const noexcept { return symbol_; }
        size_t getLine() const noexcept { return line_; }
        void* getObj() const noexcept { return obj_; }
        void unrelease() noexcept { released_ = false; }

      private:
        const int symbol_;
        void* obj_;
        size_t line_;
        bool released_ = false;
      };
    )";
}

void startDecl(ostream& out, const GrammarData& gd) {
  string type = gd.variables[1].type;
  out << "struct Start {"
         "Start("
      << type << "&& r) : r_(std::move(r)) {}" << type << " r_; };";
}

void constructObjFn(ostream& out, const GrammarData& gd) {
  out << R"(void* constructObj(int concrete, StackObj* args) {
      switch (concrete) {)";
  size_t numConcretes = gd.concretes.size();
  // Case statement for each concrete except SCONC (handled specially below)
  for (size_t i = 1; i < numConcretes; ++i) {
    const Concrete& concrete = gd.concretes[i];
    const Variable& var = gd.variables[concrete.varType];
    out << "case " << i << ": return new " << var.type << '(';

    // Catch stoi aerrors
    try {
      replacePounds(out, concrete, gd);
    } catch (const invalid_argument& e) {
      logger.logError(concrete.declLine, "Invalid argument #");
    } catch (const out_of_range& e) {
      logger.logError(concrete.declLine, "Argument # out of range of int");
    }
    out << ");";
  }

  // Root type of grammar is the first type listed
  out << "case 0: return new Start(std::move(*static_cast<" << gd.variables[1].type
      << "*>(args[0].releaseObj())));";
  out << R"(default: throw ParseException("Can't construct object. Parser programmer error.");}})";
}

void constructFn(ostream& out) {
  out << R"(StackObj construct(int concrete, std::vector<StackObj>& stk, size_t reduceStart) {
      size_t line = reduceStart == stk.size() ? 0 : stk[reduceStart].getLine();
      return StackObj(
          GRAMMAR_DATA.concretes[concrete].varType,
          constructObj(concrete, &stk[reduceStart]),
          line);
    })";
}

void constructTokenObjFn(ostream& out, const GrammarData& gd) {
  out << R"(std::optional<StackObj> constructTokenObj(int token, std::string_view str, size_t currentLine) {
      switch (token) {)";
  size_t numTokens = gd.tokens.size();
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = gd.tokens[i];
    if (token.precedence == SKIP_TOKEN) {
      out << "case " << arrIndToTok(i) << ": return {};";
    } else if (!token.type.empty()) {
      out << "case " << arrIndToTok(i) << ':'
          << "return { StackObj(token, "
             "new "
          << token.type << '(';
      ostringstream temp;
      replaceAll(temp, token.ctorExpr, "#line", "currentLine");
      replaceAll(out, temp.str(), "#str", "str");
      out << "), currentLine) }; break;";
    }
  }
  out << R"(default: return { StackObj(token, nullptr, currentLine) }; }})";
}

/********
 * DFAs *
 ********/
void lexerDFA(ostream& out, const GrammarData& gd) {
  out << "namespace lexer {";
  try {
    mergedRgxDFAToCode(out, gd);
  } catch (const regex_parser::ParseException& e) {
    logger.logError(0, e.what());
  }
  out << '}';
}

void parserDFA(ostream& out, const GrammarData& gd, const GenerateFlags& generateFlags) {
  out << "namespace parser {";
  condensedDFAToCode(out, gd, generateFlags);
  out << '}';
}

/**********
 * LEXING *
 **********/

void tokenizeFn(ostream& out) {
  out << R"(
      std::optional<StackObj> getToken(std::string_view& input, size_t& currentLine, bool& err) {
        size_t i = 0;
        const size_t len = input.size();
        size_t lastAcceptingPos;
        int lastAcceptingToken = NONE;
        size_t newlineCount = 0;
        size_t lastAcceptingNewlineCount = 0;
        lexer::Node* currentNode = lexer::root.get();

        while (currentNode) {
          int token = currentNode->v_;
          if (token != NONE) {
            lastAcceptingToken = token;
            lastAcceptingPos = i;
            lastAcceptingNewlineCount = newlineCount;
          }

          if (i == len) {
            break;
          }
          char c;
          currentNode = currentNode->step((c = input[i]));
          ++i;
          if (c == '\n') {
            ++newlineCount;
          }
        }

        if (lastAcceptingToken == NONE) {
          err = true;
          return {};
        }

        std::optional<StackObj> optStackObj =
            constructTokenObj(lastAcceptingToken, input.substr(0, lastAcceptingPos), currentLine);
        input = input.substr(lastAcceptingPos);
        currentLine += lastAcceptingNewlineCount;
        return optStackObj;
      }

      void streamTokens(std::ostream& out, std::vector<std::string> tokens) {
        size_t length = tokens.size();
        size_t i = 0;
        out << '[';

        for (const std::string& tok : tokens) {
          out << tok;
          if (i != length - 1) {
            out << ", ";
          }
          ++i;
        }
        out << ']';
      }


      std::vector<StackObj> tokenize(const std::string& input) {
        if (input.empty()) {
          return {};
        }

        std::vector<StackObj> tokens;
        std::string_view inputView = input;
        size_t currentLine = 1;

        while (!inputView.empty()) {
          bool err = false;
          std::optional<StackObj> optStackObj = getToken(inputView, currentLine, err);
          if (err) {
            std::ostringstream error;
            std::vector<std::string> prevTokenNames;
            auto startIter =
                tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
            std::transform(
                std::move(startIter),
                tokens.cend(),
                std::back_inserter(prevTokenNames),
                [](const StackObj& stackObj) { return GRAMMAR_DATA.tokens[tokToArrInd(stackObj.getSymbol())].name; });
            error << "Lexer \033[1;31merror\033[0m on line " << currentLine << " at: " << inputView.substr(0, 25) << '\n'
                << "Previous tokens were: ";
            streamTokens(error, prevTokenNames);
            throw ParseException(error.str());
          }

          if (optStackObj.has_value()) {
            tokens.push_back(std::move(*optStackObj));
          }
        }

        return tokens;
      }
    )";
}

void tokenizeFileFn(ostream& out) {
  out << R"(std::vector<StackObj> tokenize(std::istream& input) {
        return tokenize(std::string(std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}));
      }
    )";
}

void tokenizeDecl(ostream& out) {
  out << R"(
      std::vector<StackObj> tokenize(const std::string& input);
      std::vector<StackObj> tokenize(std::istream& input);
    )";
}

/***********
 * PARSING *
 ***********/

void assocDecl(ostream& out) { out << "enum class Assoc { LEFT, RIGHT, NOT, NONE };"; }

void dfaRuleDecl(ostream& out) {
  out << R"(struct DFARule {
      int concrete;
      std::vector<int> symbols;
      size_t pos;
      boost::dynamic_bitset<> lookahead;
    };
    )";
}

void ruleDataDecl(ostream& out) {
  out << R"(struct RuleData {
      DFARule reducibleRule;
      int precedence;
    };
    )";
}

void parseDecl(ostream& out, const GrammarData& gd) {
  const string& rootType = gd.variables[1].type;
  out << rootType << " parseString(const std::string& input);" << rootType
      << " parse(std::istream& input);" << rootType << " parse(const std::string& filename);";
}

void parseExceptionDecl(ostream& out) {
  out << R"(class ParseException : public std::exception {
      public:
        ParseException(const std::string& errMsg) : errMsg_(errMsg) {}
        ParseException(const char* errMsg) : errMsg_(errMsg) {}
        const char* what() const noexcept override {
          return errMsg_.c_str();
        }
      private:
        std::string errMsg_;
      };
      )";
}

void parseHelperFns(ostream& out) {
  out << R"(
      void parseError(
          std::vector<StackObj>& stk,
          const std::vector<StackObj>& inputTokens,
          size_t tokenPos) {

        std::for_each(stk.begin(), stk.end(), std::mem_fun_ref(&StackObj::unrelease));

        std::ostringstream errMsg;
        std::vector<std::string> stkSymbolNames;
        std::vector<std::string> remainingTokenNames;
        auto stkObjToName = [](const StackObj& stkObj) {
          if (isToken(stkObj.getSymbol())) {
            return GRAMMAR_DATA.tokens[tokToArrInd(stkObj.getSymbol())].name;
          }
          return GRAMMAR_DATA.variables[stkObj.getSymbol()].name;
        };

        std::transform(
            stk.begin(), stk.end(), back_inserter(stkSymbolNames), stkObjToName);
        std::transform(
            inputTokens.begin() + tokenPos,
            inputTokens.end(),
            std::back_inserter(remainingTokenNames),
            stkObjToName);

        errMsg << "Parse \033[1;31merror\033[0m on line " << stk.back().getLine() << ":\n\tStack: ";
        streamTokens(errMsg, stkSymbolNames);
        errMsg << "\n\tRemaining tokens: ";
        streamTokens(errMsg, remainingTokenNames);
        throw ParseException(errMsg.str());
      }
    )";
}

void tryReduceFn(ostream& out) {
  // If there is more than one reducible rule, then their lookahead sets are
  // disjoint (see findReduceReduceConflicts in build_parser.cpp), so we only
  // have to consider at most one rule
  out << R"(const DFARule* tryReduce(
        const parser::Node* node,
        int nextToken,
        std::vector<StackObj>& stk,
        const std::vector<StackObj>& inputTokens,
        size_t tokenPos) {
    const std::vector<RuleData>& ruleData = node->v_;
    if (ruleData.empty()) {
      return nullptr;
    }

    auto iter = find_if(ruleData.cbegin(), ruleData.cend(), [nextToken](const RuleData& rd) {
      return rd.reducibleRule.lookahead[lookaheadInd(nextToken)];
    });

    if (iter == ruleData.cend()) {
      return nullptr;
    }
    const DFARule& rule = iter->reducibleRule;

    if (!std::equal(
            rule.symbols.crbegin(),
            rule.symbols.crend(),
            stk.crbegin(),
            [](int symbol, const StackObj& stkObj) { return stkObj.getSymbol() == symbol; })) {
      return nullptr;
    }
    if (!node->ts_.contains(nextToken)) {
      return &rule;
    }

    const Token& nextTokenObj = GRAMMAR_DATA.tokens[tokToArrInd(nextToken)];
    int shiftPrecedence = nextTokenObj.precedence;

    int reducePrec = iter->precedence;
    if (reducePrec == NONE && shiftPrecedence == NONE) {
      return nullptr;
    }

    if (reducePrec > shiftPrecedence) {
      return &rule;
    }
    if (reducePrec == shiftPrecedence) {
      if (nextTokenObj.assoc == Assoc::LEFT) {
        return &rule;
      }
      if (nextTokenObj.assoc == Assoc::NOT) {
        parseError(stk, inputTokens, tokenPos);
      }
    }
    return nullptr;
  })";
} // namespace

void shiftReduceFn(ostream& out, const GrammarData& gd) {
  out << gd.variables[1].type << R"(shiftReduce(std::vector<StackObj>& inputTokens) {
        std::vector<StackObj> stk;
        std::vector<parser::Node*> dfaPath;
        size_t i = 0;
        size_t inputSize = inputTokens.size();

        while (!(i == inputSize && stk.size() == 1 && stk[0].getSymbol() == S)) {
          parser::Node* currentNode =
              stk.empty()
                  ? parser::root.get()
                  : dfaPath.back()->step(stk.back().getSymbol());
          if (currentNode == nullptr) {
            parseError(stk, inputTokens, i);
          }
          dfaPath.push_back(currentNode);

          int nextInputToken = i == inputSize ? EPSILON : inputTokens[i].getSymbol();
          const DFARule* reducibleRule =
              tryReduce(currentNode, nextInputToken, stk, inputTokens, i);
          if (reducibleRule) {
            size_t reduceStart = stk.size() - reducibleRule->symbols.size();
            StackObj newObj = construct(reducibleRule->concrete, stk, reduceStart);
            size_t stkSize = stk.size();
            for (size_t j = 0; j < stkSize - reduceStart; ++j) {
              stk.pop_back();
              dfaPath.pop_back();
            }
            stk.push_back(std::move(newObj));
          } else {
            if (nextInputToken == EPSILON) {
              parseError(stk, inputTokens, i);
            }
            stk.push_back(std::move(inputTokens[i]));
            ++i;
          }
        }

        Start* start = static_cast<Start*>(stk[0].releaseObj());
        return std::move(start->r_);
      }
    )";
}

void parseFn(ostream& out, const GrammarData& gd) {
  const string& rootType = gd.variables[1].type;
  out << rootType << R"(
      parseString(const std::string& input) {
        std::vector<StackObj> stackObjs = tokenize(input);
        return shiftReduce(stackObjs);
      }
    )" << rootType
      << R"(
      parse(std::istream& input) {
        return parseString(std::string(std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}));
      }
    )" << rootType
      << R"(
      parse(const std::string& filename) {
        std::ifstream input(filename);
        if (!input.is_open()) {
          throw std::runtime_error(std::string("Could not open file '")
              .append(filename)
              .append("': ")
              .append(strerror(errno)));
        }
        try {
          return parse(input);
        } catch (const ParseException& e) {
          throw ParseException(filename + ":\n" + e.what());
        }
      }
    )";
}

/********
 * MISC *
 ********/

void parserHppIncludes(ostream& out) {
  out << R"(
      #include <istream>
      #include <string>
    )";
}

void lexerHppIncludes(ostream& out) {
  out << R"(
      #include <istream>
      #include <string>
      #include <vector>
    )";
}

void cppIncludes(ostream& out) {
  out << R"(
      #include <string.h>

      #include <algorithm>
      #include <climits>
      #include <cstddef>
      #include <fstream>
      #include <functional>
      #include <memory>
      #include <optional>
      #include <ostream>
      #include <sstream>
      #include <stdexcept>
      #include <streambuf>
      #include <string_view>
      #include <unordered_map>
      #include <vector>

      #include <boost/dynamic_bitset.hpp>
    )";
}

void noneDecl(ostream& out) {
  out << R"(
      constexpr int NONE = INT_MIN;
    )";
}

void epsilonDecl(ostream& out) {
  out << R"(
      constexpr int EPSILON = 0;
    )";
}

void sInt(ostream& out) {
  out << R"(
      constexpr int S = 0;
    )";
}

void tokenUtils(ostream& out) {
  out << R"(
      bool isToken(int symbol) { return symbol < 0; }
      constexpr int lookaheadInd(int tokenId) noexcept { return -tokenId; }
    )";
}

void tokToArrIndFn(ostream& out) {
  out << R"(
      constexpr int tokToArrInd(int tokenId) noexcept { return -tokenId - 1; }
    )";
}

/********************
 * DRIVER FUNCTIONS *
 ********************/

constexpr char generatedWarning[] = "/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */\n";

string parserHppCode(
    const string& namespaceName, const string& headerGuard, const GenerateInfo generateInfo) {
  ostringstream out;

  out << "#ifndef " << headerGuard << "\n#define " << headerGuard << '\n' << endl;
  out << generatedWarning;
  out << generateInfo.addlHppCode;
  parserHppIncludes(out);
  out << "namespace " << namespaceName << '{';
  parseDecl(out, generateInfo.gd);
  parseExceptionDecl(out);
  out << "}\n#endif";

  return out.str();
}

string lexerHppCode(
    const string& namespaceName, const string& headerGuard, const GenerateInfo generateInfo) {
  ostringstream out;

  out << "#ifndef " << headerGuard << "\n#define " << headerGuard << '\n' << endl;
  out << generatedWarning;
  lexerHppIncludes(out);
  out << "namespace " << namespaceName << '{';
  stackObjDef(out, generateInfo.gd);
  tokenizeDecl(out);
  parseExceptionDecl(out);
  out << "}\n#endif";

  return out.str();
}

string parserCppCode(
    const string& namespaceName,
    const fs::path& includePath,
    const GenerateInfo& generateInfo,
    const GenerateFlags& generateFlags) {
  ostringstream out;
  const GrammarData& gd = generateInfo.gd;

  out << generatedWarning;
  out << "#include \"" << includePath.c_str() << "\"\n";
  cppIncludes(out);
  out << "namespace " << namespaceName << "{ namespace {" << generateInfo.addlCppCode;
  noneDecl(out);
  epsilonDecl(out);
  sInt(out);
  tokenUtils(out);
  tokToArrIndFn(out);
  assocDecl(out);
  tokenDecl(out);
  concreteDecl(out);
  variableDecl(out);
  gdDecl(out, gd);
  startDecl(out, gd);
  stackObjDef(out, gd);
  constructObjFn(out, gd);
  constructFn(out);
  constructTokenObjFn(out, gd);
  lexerDFA(out, gd);
  dfaRuleDecl(out);
  ruleDataDecl(out);
  parserDFA(out, gd, generateFlags);
  tokenizeFn(out);
  parseHelperFns(out);
  tryReduceFn(out);
  shiftReduceFn(out, gd);
  out << '}';
  parseFn(out, gd);
  out << '}';

  return out.str();
}

string lexerCppCode(
    const string& namespaceName, const fs::path& includePath, const GenerateInfo& generateInfo) {
  ostringstream out;
  const GrammarData& gd = generateInfo.gd;

  out << generatedWarning;
  out << "#include \"" << includePath.c_str() << "\"\n";
  cppIncludes(out);
  out << "namespace " << namespaceName << "{ namespace {" << generateInfo.addlCppCode;
  noneDecl(out);
  tokToArrIndFn(out);
  assocDecl(out);
  tokenDecl(out);
  concreteDecl(out);
  variableDecl(out);
  gdDecl(out, gd);
  constructTokenObjFn(out, gd);
  lexerDFA(out, gd);
  out << '}';
  tokenizeFn(out);
  tokenizeFileFn(out);
  out << '}';

  return out.str();
}

void generateCode(
    const GenerateInfo& generateInfo,
    const GenerateFlags& generateFlags,
    std::ostream& warnings,
    bool isParser) {
  fs::path filePathBase = fs::path(generateFlags.outDir) / generateFlags.name;

  fs::path hppName = filePathBase.replace_extension("hpp");
  ofstream hppFile(hppName);
  logger.checkFile(hppName.c_str(), hppFile);

  std::string cppName = filePathBase.replace_extension("cpp");
  ofstream cppFile(cppName);
  logger.checkFile(cppName.c_str(), cppFile);

  fs::path includePath =
      (fs::path(generateFlags.includePathBase) / generateFlags.name).replace_extension("hpp");
  string namespaceName = extractNamespace(includePath);
  string headerGuard = extractHeaderGuard(includePath);

  string hppCode;
  string cppCode;
  if (isParser) {
    hppCode = parserHppCode(namespaceName, headerGuard, generateInfo);
    cppCode = parserCppCode(namespaceName, includePath, generateInfo, generateFlags);
  } else {
    hppCode = lexerHppCode(namespaceName, headerGuard, generateInfo);
    cppCode = lexerCppCode(namespaceName, includePath, generateInfo);
  }

  if (logger.hasErrors()) {
    throw Logger::Exception(logger);
  }
  logger.streamLog(warnings);

  hppFile << hppCode;
  cppFile << cppCode;
}

} // namespace


void generateParserCode(
    const GenerateInfo& generateInfo, const GenerateFlags& generateFlags, std::ostream& warnings) {
  return generateCode(generateInfo, generateFlags, warnings, true);
}

void generateLexerCode(const GenerateInfo& generateInfo, const GenerateFlags& generateFlags) {
  return generateCode(generateInfo, generateFlags, cerr, false);
}
