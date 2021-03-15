#ifndef PREZ_PARSERS_COMBINATOR_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_PARSER_HPP

#include "src/parsers/combinator/execution_log.hpp"

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace prez {
namespace pcomb {


using namespace detail;

template <typename T>
struct ParseResult {
  std::optional<T> obj;
  // On success, the remaining chars. On failure, the input that was passed to the deepest failing
  // parser with an error checkpoint (if any, otherwise empty)
  std::string_view rest;
  // Name of deepest failing parser with an error checkpoint, if any
  std::optional<std::string> failedParserName;
  // TODO: Assert on error chain in tests.
  std::unique_ptr<ExecutionLog> executionLog;
};

struct ParseOptions {
  bool verbose;
};

namespace {
const std::string EMPTY_STR = "";
const ParseOptions DEFAULT_PARSE_OPTIONS = {false};
} // namespace


template <typename T>
class Parser {
public:
  using result_type = T;

  virtual ~Parser(){};

  T parse(std::string_view input, const ParseOptions& options) {
    ParseResult result = tryParse(input, options);
    if (!result.success) {
      size_t prevCharsEnd = input.size() - result.rest.size();
      size_t prevCharsBegin =
          prevCharsEnd >= NUM_PREV_CHARS_SHOWN ? prevCharsEnd - NUM_PREV_CHARS_SHOWN : 0;
      std::string_view prevChars = input.substr(prevCharsBegin, prevCharsEnd);

      std::ostringstream errMsg;
      errMsg << "Parse error: \n\t" << prevChars << " ^ "
             << result.rest.substr(0, NUM_LEFTOVER_CHARS_SHOWN) << "\n\tExpected "
             << result.failingParserName;
      throw std::runtime_error(errMsg.str());
    }

    if (!result.rest.empty()) {
      throw std::runtime_error(
          std::string("Parse error: Leftover characters: ").append(result.rest));
    }

    // TODO: log file in ParseOptions?
    if (options.verbose) {
      std::cout << *result.executionLog << std::endl;
    }

    return std::get<T>(result.obj);
  }

  ParseResult<T> tryParse(std::string_view input) {
    return tryParse(input, DEFAULT_PARSE_OPTIONS);
  };

  /**
   * obj has a value if parse succeeds
   * rest is set at the point where the deepest parser in the tree with a nonempty nameForError_
   *  failed.
   */
  virtual ParseResult<T> tryParse(std::string_view input, const ParseOptions& options) = 0;

  void setName(std::string_view name) { name_ = name; };
  std::string getName() const { return name_.empty() ? getDefaultName() : name_; }

  void setErrCheckpt() { hasErrCheckpt_ = true; }
  bool hasErrCheckpt() const { return hasErrCheckpt_; }

protected:
  template <typename... Args>
  static std::unique_ptr<ExecutionLog> makeExeLog(
      const ParseOptions& options, std::string_view input, bool success, Args&&... children) {
    return options.verbose
               ? std::make_unique<ExecutionLog>(
                     std::vector<std::unique_ptr<ExecutionLog>>{std::forward<Args>(children)...},
                     input.size(),
                     success)
               : nullptr;
  }

  virtual std::string getDefaultName() const = 0;

  std::optional<std::string> getNameForFailure() const {
    return hasErrCheckpt_ ? getName() : std::optional<std::string>();
  }

  std::string_view restIfCheckpted(std::string_view rest) { return hasErrCheckpt_ ? rest : ""; }

  std::string name_;
  bool hasErrCheckpt_ = false;

private:
  static const size_t NUM_PREV_CHARS_SHOWN = 30;
  static const size_t NUM_LEFTOVER_CHARS_SHOWN = 30;
};

} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSER_HPP
