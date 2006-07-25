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
bool CheckStringOperations()
{
  bool res = true;

  kwsys_stl::string test = "mary had a little lamb.";
  if (kwsys::SystemTools::CapitalizedWords(test) != "Mary Had A Little Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with CapitalizedWords "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::UnCapitalizedWords(test) != 
      "mary had a little lamb.")
    {
    kwsys_ios::cerr
      << "Problem with UnCapitalizedWords "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }

  test = "MaryHadTheLittleLamb.";
  if (kwsys::SystemTools::AddSpaceBetweenCapitalizedWords(test) != 
      "Mary Had The Little Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with AddSpaceBetweenCapitalizedWords "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }

  const char * cres = 
    kwsys::SystemTools::AppendStrings("Mary Had A"," Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    delete [] cres;
    kwsys_ios::cerr
      << "Problem with AppendStrings "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  cres = 
    kwsys::SystemTools::AppendStrings("Mary Had"," A ","Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    delete [] cres;
    kwsys_ios::cerr
      << "Problem with AppendStrings "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  if (kwsys::SystemTools::CountChar("Mary Had A Little Lamb.",'a') != 3)
    {
    kwsys_ios::cerr
      << "Problem with CountChar "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }

  cres = 
    kwsys::SystemTools::RemoveChars("Mary Had A Little Lamb.","aeiou");
  if (strcmp(cres,"Mry Hd A Lttl Lmb."))
    {
    delete [] cres;
    kwsys_ios::cerr
      << "Problem with RemoveChars "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  cres = 
    kwsys::SystemTools::RemoveCharsButUpperHex("Mary Had A Little Lamb.");
  if (strcmp(cres,"A"))
    {
    delete [] cres;
    kwsys_ios::cerr
      << "Problem with RemoveCharsButUpperHex "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  char *cres2 = new char [strlen("Mary Had A Little Lamb.")+1];
  strcpy(cres2,"Mary Had A Little Lamb.");
  kwsys::SystemTools::ReplaceChars(cres2,"aeiou",'X');
  if (strcmp(cres2,"MXry HXd A LXttlX LXmb."))
    {
    delete [] cres2;
    kwsys_ios::cerr
      << "Problem with ReplaceChars "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres2;

  if (!kwsys::SystemTools::StringStartsWith("Mary Had A Little Lamb.",
                                            "Mary "))
    {
    kwsys_ios::cerr
      << "Problem with StringStartsWith "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }

  if (!kwsys::SystemTools::StringEndsWith("Mary Had A Little Lamb.",
                                          " Lamb."))
    {
    kwsys_ios::cerr
      << "Problem with StringEndsWith "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }

  cres = kwsys::SystemTools::DuplicateString("Mary Had A Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    delete [] cres;
    kwsys_ios::cerr
      << "Problem with DuplicateString "
      << TEST_SYSTEMTOOLS_SRC_FILE << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::CropString(test,13) != 
      "Mary ...Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with CropString "
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

  res &= CheckStringOperations();

  return res ? 0 : 1;
}
