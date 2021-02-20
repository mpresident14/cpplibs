#include "src/generator/generate.hpp"
#include "src/generator/utils.hpp"

#include <cstddef>
#include <iostream>
#include <string>


using namespace std;

int main(int, char **argv) {
  GrammarData gd = {
      {
          {"DNOTABC", "char*", NONE, Assoc::NONE, "new char(#str[1])",
           "delete #obj;", "d[^abc]", 0},
          {"FOR", "", NONE, Assoc::NONE, "", "", "for", 0},
          {"H[ab]]LLO", "", NONE, Assoc::NONE, "", "", "h[ab\\]]llo", 0},
          {"H.LLO", "", NONE, Assoc::NONE, "", "", "h\\.llo", 0},
          {"H_LLO", "", NONE, Assoc::NONE, "", "", "h.llo", 0},
          {"STRLIT, 0", "std::string", NONE, Assoc::NONE,
           "#str.substr(1, #str.size() - 2)", "", R"("([^"]|\\")*")", 0},
          {"NUMBER", "int", NONE, Assoc::NONE, "std::stoi(std::string(#str))",
           "", "[1-9][0-9]*", 0},
          {"IDENT", "std::string", NONE, Assoc::NONE, "#str", "",
           "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])*", 0},
          {"whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+", 0},
      },
      {},
      {}};

  generateLexerCode(GenerateInfo{gd, "", ""}, GenerateFlags{argv[1], argv[2], argv[3], ""});
  return 0;
}
