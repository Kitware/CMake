/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSystemTools.h"
#include "cmWin32ProcessExecution.h"

// this is a test driver program for cmake.
int main (int argc, char *argv[])
{
  cmSystemTools::EnableMSVCDebugHook();
  if ( argc <= 1 )
    {
    std::cerr << "Usage: " << argv[0] << " executable" << std::endl;
    return 1;
    }
  std::string arg = argv[1];
  if ( (arg.find_first_of(" ") != arg.npos) &&
       (arg.find_first_of("\"") == arg.npos) )
    {
    arg = "\"" + arg + "\"";
    }
  std::string command = arg;
  int cc;
  for ( cc = 2; cc < argc; cc ++ )
    {
    std::string nextArg = argv[cc];
    if ( (nextArg.find_first_of(" ") != nextArg.npos) &&
         (nextArg.find_first_of("\"") == nextArg.npos) )
      {
      nextArg = "\"" + nextArg + "\"";
      }
    command += " ";
    command += nextArg;
    }

  return cmWin32ProcessExecution::Windows9xHack(command.c_str());
}
