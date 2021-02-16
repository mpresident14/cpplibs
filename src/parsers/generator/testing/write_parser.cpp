#include "src/parsers/generator/pgen_parse.hpp"
#include "src/parsers/generator/generate.hpp"
#include "src/parsers/generator/testing/expr_grammar.hpp"
#include "src/parsers/generator/utils.hpp"
#include "src/logger/logger.hpp"

#include <iostream>
#include <sstream>

// TODO: Download unit_test lib remotely (same for all <prez> includes)
#include <prez/unit_test.hpp>

using namespace std;
using namespace test::expr_grammar;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();
ostringstream errBuffer;

char* outDir;
char* includePath;
char* goodParserName;
char* badParserName;
const char* exprHppInclude = "#include \"src/parsers/generator/testing/expr.hpp\"\n";

void test_parserWithConflicts() {
  generateParserCode(
      ParseInfo{BAD_GRAMMAR_DATA, exprHppInclude, ""},
      ParseFlags{outDir, includePath, badParserName, ""},
      cerr);

  TESTER.assertTrue(errBuffer.str().starts_with(Logger::warningColored));
}

int main(int, char** argv) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  outDir = argv[1];
  includePath = argv[2];
  goodParserName = argv[3];
  badParserName = argv[4];

  generateParserCode(
      ParseInfo{GRAMMAR_DATA, exprHppInclude, ""},
      ParseFlags{outDir, includePath, goodParserName, ""},
      cerr);
  test_parserWithConflicts();

  return 0;
}
