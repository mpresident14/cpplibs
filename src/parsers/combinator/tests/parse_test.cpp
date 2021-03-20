#include "src/parsers/combinator/parsers.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
namespace pc = prez::pcomb;

// const char SUCCESS_OUT[] =
//     R"(
// MapTuple: 123hello456,
//   Seq: 123hello456,
//     Integer: 123, hello456
//     Greeting: hello, 456
//         hey: <FAIL> hello456
//         hello: hello, 456
//     Integer: 456,
// )";

const int SUCCESS_RESULT = 584;

// const char FAILURE_LEFTOVER_OUT[] =
//     R"(
// MapTuple: 123hello4, a56
//   Seq: 123hello4, a56
//     Integer: 123, hello4a56
//     Greeting: hello, 4a56
//         hey: <FAIL> hello4a56
//         hello: hello, 4a56
//     Integer: 4, a56
// )";

const char FAILURE_LEFTOVER_ERR[] = "Parse error: Leftover characters: a56";

// const char FAILURE_MISMATCH_OUT[] =
//     R"(
// MapTuple: <FAIL> 123bye456
//   Seq: <FAIL> 123bye456
//     Integer: 123, bye456
//     Greeting: <FAIL> bye456
//         hey: <FAIL> bye456
//         hello: <FAIL> bye456
// )";

const char FAILURE_MISMATCH_ERR[] = "Parse error on 'Greeting': 123 ^ bye456";

const auto pGreeting = pc::builder(pc::altShared<string>(pc::str("hey"), pc::str("hello")))
                           .withErrCheckpt()
                           .withName("Greeting")
                           .build();

const auto pSeq = pc::seqShared(pc::integer(), pGreeting, pc::integer());

const auto pSum =
    pc::mapTuple(pSeq, [](int m, const string& str, int n) { return m + str.size() + n; });

TEST(success) {
  int result = pSum->parse("123hello456", {true});

  assertEquals(SUCCESS_RESULT, result);
}

TEST(failure_leftover) {
  string err = assertThrows([]() { pSum->parse("123hello4a56", {true}); });

  assertContains(FAILURE_LEFTOVER_ERR, err);
}

TEST(failure_mismatched) {
  string err = assertThrows([]() { pSum->parse("123bye456", {true}); });

  assertContains(FAILURE_MISMATCH_ERR, err);
}


int main() { return runTests(); }
