#pragma once

#include <string>

namespace mathlogger {

enum LogLevel
{
  INFO,
  WARN,
  ERROR,
};

inline std::string FormatLog(LogLevel level, std::string const& message)
{
  switch (level) {
    case INFO:
      return "INFO: " + message;
    case WARN:
      return "WARN: " + message;
    case ERROR:
      return "ERROR: " + message;
  }
  return "UNKNOWN: " + message;
}

}
