#ifndef UNIT_TEST_HPP_INCLUDED
#define UNIT_TEST_HPP_INCLUDED 1

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <experimental/source_location>
#include <prez/print_stuff.hpp>

#define TEST(x)                                  \
  void x();                                      \
  int x##_dummy_var = unit_test::addTest(x, #x); \
  void x()

#define BEFORE(x)                              \
  void x();                                    \
  int x##_dummy_var = unit_test::setBefore(x); \
  void x()

#define AFTER(x)                              \
  void x();                                   \
  int x##_dummy_var = unit_test::setAfter(x); \
  void x()


namespace {

using StringSupplier = std::function<std::string(void)>;

const char* FAILURE = "\033[0;31mFAILURE\033[0m";
const char* FAILED = "\033[0;31mFAILED\033[0m";
const char* PASSED = "\033[0;32mPASSED\033[0m";

/*********************************************
 * Check to see if a type can use operator<< *
 *********************************************/

template <typename C>
concept IsPrintable = requires(std::ostream& out, const C& c) {
  out << c;
};


/**********************
 * Tracking Variables *
 **********************/

// Per test
size_t affirmsInTest_ = 0;
size_t failuresInTest_ = 0;
bool alreadyMarkedFailure_ = false;

// Per file
size_t testsFailed_ = 0;
size_t totalTests_ = 0;
size_t currentLine_ = 0;

std::unordered_map<size_t, std::string> prevLines_;
std::vector<std::pair<std::function<void(void)>, const char*>> tests_;
std::function<void(void)> before_;
std::function<void(void)> after_;

void initTest(const char* testName) {
  std::string stars(strlen(testName) + 4, '*');

  std::cout << "\n\n" << stars << '\n' << "* " << testName << " *" << '\n' << stars << '\n';

  affirmsInTest_ = 0;
  failuresInTest_ = 0;
  alreadyMarkedFailure_ = false;
  ++totalTests_;
}

// Per-test statistics
void printTestResult() {
  std::cout << "Passed " << affirmsInTest_ - failuresInTest_ << " / " << affirmsInTest_
            << " affirmations.\n\n";
}


// Entire file statistics
void showSummary(const std::vector<std::string>& lines) {
  auto iter = std::max_element(
      lines.cbegin(), lines.cend(), [](const std::string& str1, const std::string& str2) {
        return str1.size() < str2.size();
      });


  // 4 for beginning and trailing |s - 11 extra chars for colors
  std::string dashes(iter->size() - 7, '-');
  std::cout << '\n' << dashes << '\n';
  for (const std::string& line : lines) {
    if (!line.empty()) {
      std::cout << "| " << line << " |\n";
    }
  }
  std::cout << dashes << std::endl;
}

void summarizeResults() {
  std::ostringstream summary;
  summary << "SUMMARY: ";
  if (testsFailed_ == 0) {
    summary << totalTests_ - testsFailed_ << " / " << totalTests_ << " tests " << PASSED << '!';
  } else {
    summary << testsFailed_ << " / " << totalTests_ << " tests " << FAILED << '.';
  }
  showSummary({ summary.str() });
}


}  // namespace


/******************
 * Public Methods *
 ******************/
namespace unit_test {

using location = std::experimental::source_location;

void assertTrue(
    bool statement,
    const location& loc = location::current(),
    StringSupplier errSupplier = []() -> std::string { return ""; }) {
  ++affirmsInTest_;

  if (!statement) {
    ++failuresInTest_;
    std::cout << FAILURE << ": " << loc.file_name() << ", line " << loc.line() << '\n'
              << errSupplier() << '\n';
    // Update the total number of failed tests
    if (!alreadyMarkedFailure_) {
      alreadyMarkedFailure_ = true;
      ++testsFailed_;
    }
  }
}

void assertFalse(bool statement, const location& loc = location::current()) {
  assertTrue(!statement, loc);
}

template <typename T1, typename T2, std::enable_if_t<IsPrintable<T1> && IsPrintable<T2>, int> = 0>
void assertEquals(const T1& expected, const T2& actual, const location& loc = location::current()) {
  if (expected == actual) {
    return assertTrue(true, loc);
  }

  StringSupplier errSupplier = [&expected, &actual]() {
    std::ostringstream err;
    err << "\tEXPECTED:\n\t  " << expected << "\n\tGOT:\n\t  " << actual;
    return err.str();
  };
  return assertTrue(false, loc, errSupplier);
}

template <typename T1, typename T2, std::enable_if_t<!IsPrintable<T1> || !IsPrintable<T2>, int> = 0>
void assertEquals(const T1& expected, const T2& actual, const location& loc = location::current()) {
  assertTrue(expected == actual, loc);
}

template <typename T1, typename T2>
void assertNotEqual(const T1& obj, const T2& actual, const location& loc = location::current()) {
  assertTrue(obj != actual, loc);
}

void assertContains(
    std::string_view expected, std::string_view actual, const location& loc = location::current()) {
  if (actual.find(expected) != std::string_view::npos) {
    return assertTrue(true, loc);
  }

  StringSupplier errSupplier = [&expected, &actual]() {
    std::ostringstream err;
    err << "\tEXPECTED TO CONTAIN:\n\t  " << expected << "\n\tGOT:\n\t  " << actual;
    return err.str();
  };
  return assertTrue(false, loc, errSupplier);
}

template <typename F>
std::string assertThrows(const F& fn, const location& loc = location::current()) {
  try {
    fn();
    assertTrue(false, loc);
    return "";
  } catch (std::exception& e) {
    assertTrue(true, loc);
    return e.what();
  }
}

template <typename F>
int addTest(F test, const char* name) {
  tests_.emplace_back(test, name);
  // Assigning this to a dummy variable so we can call it in global space
  return 0;
}

template <typename F>
int setBefore(F beforeFn) {
  if (before_) {
    throw std::runtime_error("Multiple declarations of BEFORE function");
  }
  before_ = beforeFn;
  // Assigning this to a dummy variable so we can call it in global space
  return 0;
}

template <typename F>
int setAfter(F afterFn) {
  if (after_) {
    throw std::runtime_error("Multiple declarations of AFTER function");
  }
  after_ = afterFn;
  // Assigning this to a dummy variable so we can call it in global space
  return 0;
}

int runTests(const location& loc = location::current()) {
  for (auto& [test, testName] : tests_) {
    if (before_) before_();
    initTest(testName);
    try {
      test();
      printTestResult();
    } catch (std::exception& e) {
      if (!alreadyMarkedFailure_) {
        ++testsFailed_;
      }
      std::cout << FAILURE << ": Unexpected exception thrown: " << e.what() << "\n\n";
    }
    if (after_) after_();
  }
  summarizeResults();
  return testsFailed_ != 0;
}

}  // namespace unit_test

#endif
