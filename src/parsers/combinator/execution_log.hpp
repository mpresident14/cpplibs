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
      std::vector<std::unique_ptr<ExecutionLog>>&& pChildren, size_t pInputSize, bool pSuccess)
      : children(std::move(pChildren)), inputSize(pInputSize), success(pSuccess) {}

  std::vector<std::unique_ptr<ExecutionLog>> children;
  size_t inputSize;
  bool success;
};

std::ostream& operator<<(std::ostream& out, const ExecutionLog&) {
  return out << "TODO: Print execution log";
}


} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_EXECUTION_LOG_HPP
