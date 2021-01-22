/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#if defined(_WIN32) && !defined(CMAKE_BOOTSTRAP)
#  include "cmsys/ConsoleBuf.hxx"
#endif

class cmConsoleBuf
{
#if defined(_WIN32) && !defined(CMAKE_BOOTSTRAP)
  cmsys::ConsoleBuf::Manager m_ConsoleOut;
  cmsys::ConsoleBuf::Manager m_ConsoleErr;
#endif
public:
  cmConsoleBuf();
  ~cmConsoleBuf() = default;
  cmConsoleBuf(cmConsoleBuf const&) = delete;
  cmConsoleBuf& operator=(cmConsoleBuf const&) = delete;
  void SetUTF8Pipes();
};
