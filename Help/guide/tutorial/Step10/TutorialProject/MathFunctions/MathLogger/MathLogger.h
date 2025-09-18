#pragma once

#include <string>

#include "MathFormatting.h"
#include "MathOutput.h"

namespace mathlogger {

struct Logger
{
  LogLevel level = INFO;

  void SetLevel(LogLevel new_level) { level = new_level; }
  void Log(std::string const& message)
  {
    std::string formatted = FormatLog(level, message);
    WriteLog(formatted);
  }
};

}
