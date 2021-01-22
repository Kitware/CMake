/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConsoleBuf.h"

#if defined(_WIN32) && !defined(CMAKE_BOOTSTRAP)
cmConsoleBuf::cmConsoleBuf()
  : m_ConsoleOut(std::cout)
  , m_ConsoleErr(std::cerr, true)
{
}
#else
cmConsoleBuf::cmConsoleBuf() = default;
#endif

void cmConsoleBuf::SetUTF8Pipes()
{
#if defined(_WIN32) && !defined(CMAKE_BOOTSTRAP)
  m_ConsoleOut.SetUTF8Pipes();
  m_ConsoleErr.SetUTF8Pipes();
#endif
}
