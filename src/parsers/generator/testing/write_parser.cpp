#include "src/logger/logger.hpp"
#include "src/parsers/generator/generate.hpp"
#include "src/parsers/generator/pgen_parse.hpp"
#include "src/parsers/generator/testing/expr_grammar.hpp"
#include "src/parsers/generator/utils.hpp"
#include "src/testing/unit_test.hpp"

#include <iostream>
#include <sstream>


using namespace std;
using namespace test::expr_grammar;
using namespace prez::unit_test;


ostringstream errBuffer;

char* outDir;
char* includePath;
char* goodParserName;
char* badParserName;
const char* exprHppInclude = "#include \"src/parsers/generator/testing/expr.hpp\"\n";

TEST(parserWithConflicts) {
  generateParserCode(
      GenerateInfo{BAD_GRAMMAR_DATA, exprHppInclude, ""},
      GenerateFlags{outDir, includePath, badParserName, ""},
      cerr);

  assertTrue(errBuffer.str().starts_with(Logger::warningColored));
}

int main(int, char** argv) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  outDir = argv[1];
  includePath = argv[2];
  goodParserName = argv[3];
  badParserName = argv[4];

  generateParserCode(
      GenerateInfo{GRAMMAR_DATA, exprHppInclude, ""},
      GenerateFlags{outDir, includePath, goodParserName, ""},
      cerr);
  runTests();

  return 0;
}
