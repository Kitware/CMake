/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemTools.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

const char* toUnixPaths[][2] = 
{
    { "/usr/local/bin/passwd", "/usr/local/bin/passwd" },
    { "/usr/lo cal/bin/pa sswd", "/usr/lo cal/bin/pa sswd" },
    { "/usr/lo\\ cal/bin/pa\\ sswd", "/usr/lo\\ cal/bin/pa\\ sswd" },
    { "c:/usr/local/bin/passwd", "c:/usr/local/bin/passwd" },
    { "c:/usr/lo cal/bin/pa sswd", "c:/usr/lo cal/bin/pa sswd" },
    { "c:/usr/lo\\ cal/bin/pa\\ sswd", "c:/usr/lo\\ cal/bin/pa\\ sswd" },
    { "\\usr\\local\\bin\\passwd", "/usr/local/bin/passwd" },
    { "\\usr\\lo cal\\bin\\pa sswd", "/usr/lo cal/bin/pa sswd" },
    { "\\usr\\lo\\ cal\\bin\\pa\\ sswd", "/usr/lo\\ cal/bin/pa\\ sswd" },
    { "c:\\usr\\local\\bin\\passwd", "c:/usr/local/bin/passwd" },
    { "c:\\usr\\lo cal\\bin\\pa sswd", "c:/usr/lo cal/bin/pa sswd" },
    { "c:\\usr\\lo\\ cal\\bin\\pa\\ sswd", "c:/usr/lo\\ cal/bin/pa\\ sswd" },
    { "\\\\usr\\local\\bin\\passwd", "//usr/local/bin/passwd" },
    { "\\\\usr\\lo cal\\bin\\pa sswd", "//usr/lo cal/bin/pa sswd" },
    { "\\\\usr\\lo\\ cal\\bin\\pa\\ sswd", "//usr/lo\\ cal/bin/pa\\ sswd" },
    {0, 0}
};

bool CheckConvertToUnixSlashes(kwsys_stl::string input, kwsys_stl::string output)
{
  kwsys_stl::string result = input;
  kwsys::SystemTools::ConvertToUnixSlashes(result);
  if ( result != output )
    {
    kwsys_ios::cerr << "Problem with ConvertToUnixSlashes - input: " << input.c_str() << " output: " << result.c_str() << " expected: " << output.c_str() << kwsys_ios::endl;
    return false;
    }
  return true;
}

int main(/*int argc, char* argv*/)
{
  int cc;
  for ( cc = 0; toUnixPaths[cc][0]; cc ++ )
    {
    CheckConvertToUnixSlashes(toUnixPaths[cc][0], toUnixPaths[cc][1]);
    }

  // Special check for ~
  kwsys_stl::string output;
  if(kwsys::SystemTools::GetEnv("HOME", output))
    {
    output += "/foo bar/lala";
    CheckConvertToUnixSlashes("~/foo bar/lala", output);
    }
  return 0;
}
