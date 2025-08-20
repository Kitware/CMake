#pragma once

#include <iostream>
#include <string>

namespace mathlogger {
inline void WriteLog(std::string const& msg)
{
  std::cout << msg;
}
}
