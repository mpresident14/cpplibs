#include "src/logger/logger.hpp"


using namespace std;


Logger::Logger(string_view filename) : filename_(filename) {}

ostringstream& Logger::logError(size_t line, string_view msg) {
  hasErrors_ = true;
  return log(MsgType::ERROR, line, msg);
}

ostringstream& Logger::logWarning(size_t line, string_view msg) {
  return log(MsgType::WARNING, line, msg);
}

ostringstream& Logger::logNote(size_t line, string_view msg) {
  return log(MsgType::NOTE, line, msg);
}

ostringstream& Logger::log(MsgType msgType, size_t line, string_view msg) {
  logs_.push_back({ msgType, line, ostringstream() });
  ostringstream& error = logs_.back().msg;
  error << msg;
  return error;
}

void Logger::logFatal(size_t line, string_view msg) {
  logError(line, msg);
  throw Exception(*this);
}


bool Logger::hasErrors() const noexcept { return hasErrors_; }


void Logger::streamLog(ostream& out) const {
  if (logs_.empty()) {
    return;
  }

  if (!filename_.empty()) {
    out << filename_ << ":\n";
  }

  MsgCounts counts;
  for (const MsgInfo& msgInfo : sortLogs()) {
    streamMsg(out, msgInfo, counts);
  }

  streamCounts(out, counts);
}

std::set<Logger::MsgInfo> Logger::sortLogs() const {
  std::set<MsgInfo> logSet;
  for (const auto& [msgType, line, msg] : logs_) {
    logSet.insert({ msgType, line, msg.str() });
  }
  return logSet;
}

bool operator<(const Logger::MsgInfo& mi1, const Logger::MsgInfo& mi2) noexcept {
  auto lineCmp = mi1.line <=> mi2.line;
  auto typeCmp = static_cast<int>(mi1.msgType) <=> static_cast<int>(mi2.msgType);
  if (lineCmp == 0) {
    if (typeCmp == 0) {
      return mi1.msg < mi2.msg;
    } else {
      return typeCmp < 0;
    }
  } else {
    return lineCmp < 0;
  }
}


void Logger::streamMsg(std::ostream& out, const MsgInfo& mi, MsgCounts& counts) {
  switch (mi.msgType) {
    case MsgType::ERROR:
      ++counts.errors;
      out << errorColored;
      break;
    case MsgType::WARNING:
      ++counts.warnings;
      out << warningColored;
      break;
    case MsgType::NOTE:
      ++counts.notes;
      out << noteColored;
      break;
    default:
      throw invalid_argument("Unknown MsgType");
  }
  if (mi.line != 0) {
    out << " on line " << mi.line;
  }
  out << ": " << mi.msg << '\n';
}

namespace {

const char* maybePlural(size_t n, const char* singular, const char* plural) {
  return n == 1 ? singular : plural;
}

}  // namespace

void Logger::streamCounts(std::ostream& out, const MsgCounts& counts) {
  out << counts.errors << maybePlural(counts.errors, " error, ", " errors, ") << counts.warnings
      << maybePlural(counts.warnings, " warning, ", " warnings, ") << counts.notes
      << maybePlural(counts.notes, " note\n\n", " notes\n\n");
}


Logger::Exception::Exception(const Logger& logger) {
  if (!logger.filename_.empty()) {
    what_.append(logger.filename_).append(":\n");
  }

  ostringstream msg;
  MsgCounts counts;
  for (const MsgInfo& msgInfo : logger.sortLogs()) {
    Logger::streamMsg(msg, msgInfo, counts);
  }
  what_.append(msg.str());
}

const char* Logger::Exception::what() const noexcept { return what_.c_str(); }
