#ifndef PREZ_PARSERS_COMBINATOR_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_PARSER_HPP

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace prez {
namespace pcomb {

template <typename T>
struct ParseResult {
  bool success;
  // TODO: Assert on error chain in tests.
  std::variant<T, std::vector<std::string>> objOrErrorChain;
  std::string_view rest;
};


template <typename T>
class Parser {
public:
  using result_type = T;

  virtual ~Parser(){};

  T parse(std::string_view input) {
    ParseResult result = tryParse(input);
    if (!result.success) {
      size_t prevCharsEnd = input.size() - result.rest.size();
      size_t prevCharsBegin =
          prevCharsEnd >= NUM_PREV_CHARS_SHOWN ? prevCharsEnd - NUM_PREV_CHARS_SHOWN : 0;
      std::string_view prevChars = input.substr(prevCharsBegin, prevCharsEnd);

      std::ostringstream errMsg;
      errMsg << "Parse error: \n\t" << prevChars << " ^ "
             << result.rest.substr(0, NUM_LEFTOVER_CHARS_SHOWN) << "\n\tExpected "
             << std::get<std::vector<std::string>>(result.objOrErrorChain);
      throw std::runtime_error(errMsg.str());
    }

    if (!result.rest.empty()) {
      throw std::runtime_error(
          std::string("Parse error: Leftover characters: ").append(result.rest));
    }

    return std::get<T>(result.obj);
  }

  /**
   * obj has a value if parse succeeds
   * rest is set at the point where the deepest parser in the tree with a nonempty nameForError_
   *  failed.
   */
  virtual ParseResult<T> tryParse(std::string_view input) = 0;

  void setName(std::string_view name) { name_ = name; };
  const std::string& getName() const { return name_.empty() ? getDefaultName() : name_; }

  // TODO: Rename this method
  void setErrCheckpt() { hasErrCheckpt_ = true; }
  bool hasErrCheckpt() const { return hasErrCheckpt_; }

  virtual std::string getErrorChain() const { return ""; }

protected:
  virtual const std::string& getDefaultName() const = 0;

  std::string name_;
  bool hasErrCheckpt_ = false;

private:
  static const size_t NUM_PREV_CHARS_SHOWN = 30;
  static const size_t NUM_LEFTOVER_CHARS_SHOWN = 30;
};

} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSER_HPP
