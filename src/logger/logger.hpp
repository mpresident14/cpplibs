#ifndef PREZ_LOGGER_LOGGER_HPP
#define PREZ_LOGGER_LOGGER_HPP

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <string.h>

class Logger {
public:
  class Exception : public std::exception {
  public:
    Exception(const Logger& logger);
    const char* what() const noexcept override;

  private:
    std::string what_;
  };

  friend class Exception;

  enum class MsgType { ERROR, WARNING, NOTE };

  static constexpr char errorColored[] = "\033[1;31mError\033[0m";
  static constexpr char warningColored[] = "\033[1;35mWarning\033[0m";
  static constexpr char noteColored[] = "\033[1;34mNote\033[0m";

  Logger() = default;
  Logger(std::string_view filename);
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger(Logger&&) = default;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = default;

  std::ostringstream& logError(size_t line = 0, std::string_view msg = "");
  std::ostringstream& logWarning(size_t line = 0, std::string_view msg = "");
  std::ostringstream& logNote(size_t line = 0, std::string_view msg = "");
  void logFatal(size_t line, std::string_view msg);

  template <typename FStream>
  void checkFile(std::string_view filename, FStream& file) {
    if (!file.is_open()) {
      logFatal(
          0,
          std::string("Could not open file '")
              .append(filename)
              .append("': ")
              .append(strerror(errno)));
    }
  }

  bool hasErrors() const noexcept;
  void streamLog(std::ostream& out = std::cerr) const;


private:
  struct MsgInfo {
    MsgType msgType;
    size_t line;
    std::string msg;
  };
  friend bool operator<(const MsgInfo& mi1, const MsgInfo& mi2) noexcept;

  struct MsgCounts {
    size_t errors = 0;
    size_t warnings = 0;
    size_t notes = 0;
  };
  static void streamMsg(std::ostream& out, const MsgInfo& mi, MsgCounts& counts);
  static void streamCounts(std::ostream& out, const MsgCounts& counts);

  struct MsgInfoStream {
    MsgType msgType;
    size_t line;
    std::ostringstream msg;
  };
  std::set<MsgInfo> sortLogs() const;
  std::ostringstream& log(MsgType msgType, size_t line, std::string_view msg);

  std::vector<MsgInfoStream> logs_;
  std::string filename_ = "";
  bool hasErrors_ = false;
};


#endif // LOGGER_HPP
