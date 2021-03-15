#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;

auto P_HEY = pcomb::strShared("hey");
auto P_HELLO = pcomb::strShared("hello");


TEST(success_exact_first) {
  auto p = pcomb::alt<string>(P_HEY, P_HELLO);
  string expected = "hey";

  auto result = p->tryParse("hey");

  assertParseResult(result, expected, "");
}

TEST(success_exact_second) {
  auto p = pcomb::alt<string>(P_HEY, P_HELLO);
  string expected = "hello";

  auto result = p->tryParse("hello");

  assertParseResult(result, expected, "");
}

TEST(success_leftover_first_verbose) {
  auto p = pcomb::create(pcomb::alt<string>(P_HEY, P_HELLO)).withErrCheckpt().build();
  string expected = "hey";

  auto result = p->tryParse("heyyo", {true});

  assertParseResult(result, expected, "yo");
  // Short-circuits, so P_HELLO is not executed.
  verifyExecLog(result, true, 5, 1);
}

TEST(success_leftover_second_verbose) {
  auto p = pcomb::create(pcomb::alt<string>(P_HEY, P_HELLO)).withErrCheckpt().build();
  string expected = "hello";

  auto result = p->tryParse("helloyo", {true});

  assertParseResult(result, expected, "yo");
  verifyExecLog(result, true, 7, 2);
}

TEST(success_one) {
  auto p = pcomb::alt<int>(pcomb::integer());
  auto expected = 12345;

  auto result = p->tryParse("12345a");

  assertParseResult(result, expected, "a");
}

TEST(success_many) {
  auto p = pcomb::alt<string>(P_HELLO, P_HEY, pcomb::str("abc"), pcomb::str("yo"));
  string expected = "abc";

  auto result = p->tryParse("abc");

  assertParseResult(result, expected, "");
}

TEST(success_resultsAreConverted) {
  auto p = pcomb::alt<optional<string>>(P_HELLO, P_HEY);
  optional<string> expected = "hello";

  auto result = p->tryParse("hello");

  assertParseResult(result, expected, "");
}

TEST(success_assumesOwnershipOfParsers) {
  // Making sure that alt parser is assuming ownership of the parser ptrs it is passed.
  auto makeAlt = []() { return pcomb::alt<string>(P_HEY, pcomb::str("hello")); };
  auto p = makeAlt();
  string expected = "hello";

  auto result = p->tryParse("hello123");

  assertParseResult(result, expected, "123");
}

TEST(failure_empty) {
  auto p = pcomb::alt<int>();

  auto result = p->tryParse("123");

  assertEmptyParseResult(result, "");
}

TEST(failure_withErrCheckpt_verbose) {
  auto p = pcomb::create(pcomb::alt<string>(P_HEY, P_HELLO)).withErrCheckpt().build();

  auto result = p->tryParse("123", {true});

  assertEmptyParseResult(result, "123", "Alt");
  verifyExecLog(result, false, 3, 2);
}

TEST(failure_withErrCheckpt_subparserHasErrCheckpt_truncatesRest) {
  auto pSeqWithCheckpt =
      pcomb::seq(pcomb::str("hi"), pcomb::create(pcomb::str("yo")).withErrCheckpt().build());
  auto pSeqNoCheckpt = pcomb::seq(P_HEY, P_HELLO);
  auto p = pcomb::create(pcomb::alt<tuple<string, string>>(
                             std::move(pSeqWithCheckpt), std::move(pSeqNoCheckpt)))
               .withErrCheckpt()
               .build();

  auto result = p->tryParse("hithere");
  assertEmptyParseResult(result, "there", "\"yo\"");
}


int main() { return runTests(); }
