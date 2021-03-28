#include "src/parsers/combinator/parsers.hpp"
#include "src/testing/unit_test.hpp"
#include "src/testing/utils.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez::testing;
namespace pc = prez::pcomb;


const auto pGreeting = pc::builder(pc::altShared<string>(pc::str("hey"), pc::str("hello")))
                           .withErrCheckpt()
                           .withName("Greeting")
                           .build();

const auto pSeq = pc::seqShared(pc::integer(), pGreeting, pc::integer());

const auto pSum =
    pc::mapTuple(pSeq, [](int m, const string& str, int n) { return m + str.size() + n; });

TEST(success) {
  const char SUCCESS_OUT[] = "MapTuple: 123hello456, \n"
                             "  Seq: 123hello456, \n"
                             "    Integer: 123, hello456\n"
                             "    Greeting: hello, 456\n"
                             "      hey: <FAIL> hello456\n"
                             "      hello: hello, 456\n"
                             "    Integer: 456, \n";
  const int SUCCESS_RESULT = 584;
  string captureOut;

  int result = captureStdOut(captureOut, []() { return pSum->parse("123hello456", {true}); });

  assertEquals(SUCCESS_RESULT, result);
  assertContains(SUCCESS_OUT, captureOut);
}

TEST(failure_leftover) {
  const char FAILURE_LEFTOVER_OUT[] = "MapTuple: 123hello4, a56\n"
                                      "  Seq: 123hello4, a56\n"
                                      "    Integer: 123, hello4a56\n"
                                      "    Greeting: hello, 4a56\n"
                                      "      hey: <FAIL> hello4a56\n"
                                      "      hello: hello, 4a56\n"
                                      "    Integer: 4, a56\n";
  const char FAILURE_LEFTOVER_ERR[] = "Parse error: Leftover characters: a56";
  string captureOut;

  string err = assertThrows([&captureOut]() {
    captureStdOut(captureOut, []() { return pSum->parse("123hello4a56", {true}); });
  });

  assertContains(FAILURE_LEFTOVER_ERR, err);
  assertContains(FAILURE_LEFTOVER_OUT, captureOut);
}

TEST(failure_mismatched) {
  const char FAILURE_MISMATCH_OUT[] = "MapTuple: <FAIL> 123bye456\n"
                                      "  Seq: <FAIL> 123bye456\n"
                                      "    Integer: 123, bye456\n"
                                      "    Greeting: <FAIL> bye456\n"
                                      "      hey: <FAIL> bye456\n"
                                      "      hello: <FAIL> bye456\n";
  const char FAILURE_MISMATCH_ERR[] = "Parse error on 'Greeting': 123 ^ bye456";
  string captureOut;

  string err = assertThrows([&captureOut]() {
    captureStdOut(captureOut, []() { return pSum->parse("123bye456", {true}); });
  });

  assertContains(FAILURE_MISMATCH_ERR, err);
  assertContains(FAILURE_MISMATCH_OUT, captureOut);
}


int main() { return runTests(); }
