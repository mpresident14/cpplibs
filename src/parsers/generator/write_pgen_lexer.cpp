#include "src/parsers/generator/pgen_grammar.hpp"
#include "src/parsers/generator/generate.hpp"
#include "src/parsers/generator/utils.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int, char** argv) {
  generateLexerCode(argv[1], argv[2], argv[3], ADDL_CODE, CONFIG_GRAMMAR);
  return 0;
}
