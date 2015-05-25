/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmRegexTools.h"

#include <cmsys/RegularExpression.hxx>

#include <vector>

//----------------------------------------------------------------------------
namespace
{
  class RegexReplacement
  {
  public:
    RegexReplacement(const char* s): number(-1), value(s) {}
    RegexReplacement(const std::string& s): number(-1), value(s) {}
    RegexReplacement(int n): number(n), value() {}
    RegexReplacement() {}
    int number;
    std::string value;
  };
}
//----------------------------------------------------------------------------
std::string RegexReplace(std::string const& input, std::string const& regex, std::string const& replace)
{
  // Pull apart the replace expression to find the escaped [0-9] values.
  std::vector<RegexReplacement> replacement;
  std::string::size_type l = 0;
  while(l < replace.length())
    {
    std::string::size_type r = replace.find("\\", l);
    if(r == std::string::npos)
      {
      r = replace.length();
      replacement.push_back(replace.substr(l, r-l));
      }
    else
      {
      if(r-l > 0)
        {
        replacement.push_back(replace.substr(l, r-l));
        }
      if(r == (replace.length()-1))
        {
//        this->SetError("sub-command REGEX, mode REPLACE: "
//                       "replace-expression ends in a backslash.");
        return input;
        }
      if((replace[r+1] >= '0') && (replace[r+1] <= '9'))
        {
        replacement.push_back(replace[r+1]-'0');
        }
      else if(replace[r+1] == 'n')
        {
        replacement.push_back("\n");
        }
      else if(replace[r+1] == '\\')
        {
        replacement.push_back("\\");
        }
      else
        {
        std::string e = "sub-command REGEX, mode REPLACE: Unknown escape \"";
        e += replace.substr(r, 2);
        e += "\" in replace-expression.";
//        this->SetError(e);
        return input;
        }
      r += 2;
      }
    l = r;
    }

  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e =
      "sub-command REGEX, mode REPLACE failed to compile regex \""+
      regex+"\".";
//    this->SetError(e);
    return input;
    }

  // Scan through the input for all matches.
  std::string output;
  std::string::size_type base = 0;
  while(re.find(input.c_str()+base))
    {
//    this->Makefile->StoreMatches(re);
    std::string::size_type l2 = re.start();
    std::string::size_type r = re.end();

    // Concatenate the part of the input that was not matched.
    output += input.substr(base, l2);

    // Make sure the match had some text.
    if(r-l2 == 0)
      {
      std::string e = "sub-command REGEX, mode REPLACE regex \""+
        regex+"\" matched an empty string.";
//      this->SetError(e);
      return input;
      }

    // Concatenate the replacement for the match.
    for(unsigned int i=0; i < replacement.size(); ++i)
      {
      if(replacement[i].number < 0)
        {
        // This is just a plain-text part of the replacement.
        output += replacement[i].value;
        }
      else
        {
        // Replace with part of the match.
        int n = replacement[i].number;
        std::string::size_type start = re.start(n);
        std::string::size_type end = re.end(n);
        std::string::size_type len = input.length()-base;
        if((start != std::string::npos) && (end != std::string::npos) &&
           (start <= len) && (end <= len))
          {
          output += input.substr(base+start, end-start);
          }
        else
          {
          std::string e =
            "sub-command REGEX, mode REPLACE: replace expression \""+
            replace+"\" contains an out-of-range escape for regex \""+
            regex+"\".";
//          this->SetError(e);
          return input;
          }
        }
      }

    // Move past the match.
    base += r;
    }

  // Concatenate the text after the last match.
  output += input.substr(base, input.length()-base);

  return output;
}
