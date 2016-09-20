/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2016 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"

#include KWSYS_HEADER(ConsoleBuf.hxx)
#include KWSYS_HEADER(Encoding.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "ConsoleBuf.hxx.in"
# include "Encoding.hxx.in"
#endif

#include <iostream>
#include "testConsoleBuf.hxx"

//----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
#if defined(_WIN32)
  kwsys::ConsoleBuf::Manager out(std::cout);
  kwsys::ConsoleBuf::Manager err(std::cerr, true);
  kwsys::ConsoleBuf::Manager in(std::cin);

  if (argc > 1) {
    std::cout << argv[1] << std::endl;
    std::cerr << argv[1] << std::endl;
  } else {
    std::string str = kwsys::Encoding::ToNarrow(UnicodeTestString);
    std::cout << str << std::endl;
    std::cerr << str << std::endl;
  }

  std::string input;
  HANDLE event = OpenEventW(EVENT_MODIFY_STATE, FALSE, BeforeInputEventName);
  if (event) {
    SetEvent(event);
    CloseHandle(event);
  }

  std::cin >> input;
  std::cout << input << std::endl;
  event = OpenEventW(EVENT_MODIFY_STATE, FALSE, AfterOutputEventName);
  if (event) {
    SetEvent(event);
    CloseHandle(event);
  }
#else
  static_cast<void>(argc);
  static_cast<void>(argv);
#endif
  return 0;
}
