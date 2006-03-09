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

#if defined(_MSC_VER)
# pragma warning (disable:4786)
#endif

#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemTools.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

#include "testSystemTools.h"

//----------------------------------------------------------------------------
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

bool CheckConvertToUnixSlashes(kwsys_stl::string input,
                               kwsys_stl::string output)
{
  kwsys_stl::string result = input;
  kwsys::SystemTools::ConvertToUnixSlashes(result);
  if ( result != output )
    {
    kwsys_ios::cerr
      << "Problem with ConvertToUnixSlashes - input: " << input.c_str()
      << " output: " << result.c_str() << " expected: " << output.c_str()
      << kwsys_ios::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
const char* checkEscapeChars[][4] =
{
  { "1 foo 2 bar 2", "12", "\\", "\\1 foo \\2 bar \\2"},
  { " {} ", "{}", "#", " #{#} "},
  {0, 0, 0, 0}
};

bool CheckEscapeChars(kwsys_stl::string input,
                      const char *chars_to_escape,
                      char escape_char,
                      kwsys_stl::string output)
{
  kwsys_stl::string result = kwsys::SystemTools::EscapeChars(
    input.c_str(), chars_to_escape, escape_char);
  if (result != output)
    {
    kwsys_ios::cerr
      << "Problem with CheckEscapeChars - input: " << input.c_str()
      << " output: " << result.c_str() << " expected: " << output.c_str()
      << kwsys_ios::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckDetectFileType()
{
  bool res = true;

  if (kwsys::SystemTools::DetectFileType(TEST_SYSTEMTOOLS_BIN_FILE) !=
      kwsys::SystemTools::FileTypeBinary)
    {
    kwsys_ios::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << TEST_SYSTEMTOOLS_BIN_FILE << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::DetectFileType(TEST_SYSTEMTOOLS_SRC_FILE) !=
      kwsys::SystemTools::FileTypeText)
    {
    kwsys_ios::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;
    }

  return res;
}

//----------------------------------------------------------------------------
int main(/*int argc, char* argv*/)
{
  bool res = true;

  int cc;
  for ( cc = 0; toUnixPaths[cc][0]; cc ++ )
    {
    res &= CheckConvertToUnixSlashes(toUnixPaths[cc][0], toUnixPaths[cc][1]);
    }

  // Special check for ~
  kwsys_stl::string output;
  if(kwsys::SystemTools::GetEnv("HOME", output))
    {
    output += "/foo bar/lala";
    res &= CheckConvertToUnixSlashes("~/foo bar/lala", output);
    }

  for (cc = 0; checkEscapeChars[cc][0]; cc ++ )
    {
    res &= CheckEscapeChars(checkEscapeChars[cc][0], checkEscapeChars[cc][1], 
                            *checkEscapeChars[cc][2], checkEscapeChars[cc][3]);
    }

  res &= CheckDetectFileType();

  return res ? 0 : 1;
}
