#ifndef PREZ_PARSERS_GENERATOR_PGEN_GRAMMAR_HPP
#define PREZ_PARSERS_GENERATOR_PGEN_GRAMMAR_HPP

#include "src/parsers/generator/utils.hpp"

enum TokenId {
  HEADER = -1,
  SOURCE = -2,
  TOKENS = -3,
  SKIP = -4,
  PREC = -5,
  GRAMMAR = -6,
  LEFTASSOC = -7,
  RIGHTASSOC = -8,
  NONASSOC = -9,
  DEFINED = -10,
  BAR = -11,
  ARROW = -12,
  COMMENT = -13,
  IDENT = -14,
  TYPE = -15,
  STRLIT = -16,
  CODE = -17,
};

GrammarData CONFIG_GRAMMAR = {
    {
        {"#header", "", NONE, Assoc::NONE, "", "", "#header", 0},
        {"#source", "", NONE, Assoc::NONE, "", "", "#source", 0},
        {"#tokens", "", NONE, Assoc::NONE, "", "", "#tokens", 0},
        {"#skip", "", NONE, Assoc::NONE, "", "", "#skip", 0},
        {"#prec", "", NONE, Assoc::NONE, "", "", "#prec", 0},
        {"#grammar", "", NONE, Assoc::NONE, "", "", "#grammar", 0},
        {"associativity marker", "", NONE, Assoc::NONE, "", "", "#left", 0},
        {"associativity marker", "", NONE, Assoc::NONE, "", "", "#right", 0},
        {"associativity marker", "", NONE, Assoc::NONE, "", "", "#nonassoc", 0},
        {":=", "", NONE, Assoc::NONE, "", "", ":=", 0},
        {"|", "", NONE, Assoc::NONE, "", "", "\\|", 0},
        {"->", "", NONE, Assoc::NONE, "", "", "->", 0},
        {"COMMENT", "", SKIP_TOKEN, Assoc::NONE, "", "", "//[^\n]*\n", 0},
        {"identifier",
         "std::string",
         NONE,
         Assoc::NONE,
         "#str",
         "",
         "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|_)*",
         0},
        {"type", "std::string", NONE, Assoc::NONE, "rmEnclosure(#str)", "", "{[^}]*}", 0},
        {"string literal",
         "std::string",
         NONE,
         Assoc::NONE,
         "extractString(#str, '\"')",
         "",
         R"("([^"]|\\")*")",
         0},
        {"code block",
         "std::string",
         NONE,
         Assoc::NONE,
         "extractString(#str, '%')",
         "",
         R"(%([^%]|\\%)*%)",
         0},
        {"whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+", 0},
    },
    {},
    {}};

const char ADDL_CODE[] =
    R"(
      using namespace std;

      string rmEnclosure(string_view str) {
        return string(str.substr(1, str.size() - 2));
      }

      string extractString(string_view str, char escaped) {
        string ret;
        string_view lit = str.substr(1, str.size() - 2);
        if (lit.empty()) {
          return ret;
        }
        size_t len = lit.size();
        ret.reserve(len);
        for (size_t i = 0; i < len - 1; ++i) {
          char c = lit[i];
          if (c == '\\' && lit[i+1] == escaped) {
            continue;
          }
          ret.append(1, c);
        }
        ret.append(1, lit.back());
        return ret;
      }
    )";

#endif
