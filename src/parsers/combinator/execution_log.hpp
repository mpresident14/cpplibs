#ifndef PREZ_PARSERS_COMBINATOR_EXECUTION_LOG_HPP
#define PREZ_PARSERS_COMBINATOR_EXECUTION_LOG_HPP

#include <memory>
#include <vector>

namespace prez {
namespace pcomb {
namespace detail {

struct ExecutionLog {
  // For std::make_unique
  ExecutionLog(
      std::vector<std::unique_ptr<ExecutionLog>>&& pChildren,
      std::string_view pParserName,
      std::string_view pInput,
      std::string_view pRest,
      bool pSuccess)
      : children(std::move(pChildren)), parserName(pParserName), input(pInput), rest(pRest),
        success(pSuccess) {}


  std::vector<std::unique_ptr<ExecutionLog>> children;
  std::string parserName;
  std::string_view input;
  std::string_view rest;
  bool success;
};

std::ostream& operator<<(std::ostream& out, const ExecutionLog& execLog) {
  struct StackObj {
    const ExecutionLog* logPtr;
    size_t depth;
  };

  std::vector<StackObj> logStack = {{&execLog, 0}};
  while (!logStack.empty()) {
    const StackObj& stackObj = logStack.back();
    size_t depth = stackObj.depth;
    out << std::string(depth * 2, ' ');
    const ExecutionLog* logPtr = stackObj.logPtr;
    out << logPtr->parserName << ": ";
    if (logPtr->success) {
      out << logPtr->input.substr(0, logPtr->input.size() - logPtr->rest.size()) << ", "
          << logPtr->rest;
    } else {
      out << "<FAIL> " << logPtr->input;
    }
    out << '\n';

    logStack.pop_back();
    for (auto iter = logPtr->children.crbegin(); iter != logPtr->children.crend(); ++iter) {
      logStack.push_back({iter->get(), depth + 1});
    }
  }
  return out;
}


} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_EXECUTION_LOG_HPP
