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
#include KWSYS_HEADER(Glob.hxx)

#include KWSYS_HEADER(Configure.hxx)

#include KWSYS_HEADER(RegularExpression.hxx)
#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(Directory.hxx)
#include KWSYS_HEADER(stl/string)
#include KWSYS_HEADER(stl/vector)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Glob.hxx.in"
# include "Directory.hxx.in"
# include "Configure.hxx.in"
# include "RegularExpression.hxx.in"
# include "SystemTools.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_stl_string.hxx.in"
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
namespace KWSYS_NAMESPACE
{
#if defined(_WIN32) || defined(APPLE) || defined(__CYGWIN__)
// On Windows and apple, no difference between lower and upper case
# define KWSYS_GLOB_CASE_INDEPENDENT
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
// Handle network paths
# define KWSYS_GLOB_SUPPORT_NETWORK_PATHS
#endif

//----------------------------------------------------------------------------
class GlobInternals
{
public:
  kwsys_stl::vector<kwsys_stl::string> Files;
  kwsys_stl::vector<kwsys::RegularExpression> Expressions;
};

//----------------------------------------------------------------------------
Glob::Glob()
{
  this->Internals = new GlobInternals;
  this->Recurse = false;
  this->Relative = "";
}

//----------------------------------------------------------------------------
Glob::~Glob()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
kwsys_stl::vector<kwsys_stl::string>& Glob::GetFiles()
{
  return this->Internals->Files;
}

//----------------------------------------------------------------------------
kwsys_stl::string Glob::PatternToRegex(const kwsys_stl::string& pattern,
                                       bool require_whole_string)
{
  // Incrementally build the regular expression from the pattern.
  kwsys_stl::string regex = require_whole_string? "^" : "";
  kwsys_stl::string::const_iterator pattern_first = pattern.begin();
  kwsys_stl::string::const_iterator pattern_last = pattern.end();
  for(kwsys_stl::string::const_iterator i = pattern_first;
      i != pattern_last; ++i)
    {
    int c = *i;
    if(c == '*')
      {
      // A '*' (not between brackets) matches any string.
      regex += ".*";
      }
    else if(c == '?')
      {
      // A '?' (not between brackets) matches any single character.
      regex += ".";
      }
    else if(c == '[')
      {
      // Parse out the bracket expression.  It begins just after the
      // opening character.
      kwsys_stl::string::const_iterator bracket_first = i+1;
      kwsys_stl::string::const_iterator bracket_last = bracket_first;

      // The first character may be complementation '!' or '^'.
      if(bracket_last != pattern_last &&
         (*bracket_last == '!' || *bracket_last == '^'))
        {
        ++bracket_last;
        }

      // If the next character is a ']' it is included in the brackets
      // because the bracket string may not be empty.
      if(bracket_last != pattern_last && *bracket_last == ']')
        {
        ++bracket_last;
        }

      // Search for the closing ']'.
      while(bracket_last != pattern_last && *bracket_last != ']')
        {
        ++bracket_last;
        }

      // Check whether we have a complete bracket string.
      if(bracket_last == pattern_last)
        {
        // The bracket string did not end, so it was opened simply by
        // a '[' that is supposed to be matched literally.
        regex += "\\[";
        }
      else
        {
        // Convert the bracket string to its regex equivalent.
        kwsys_stl::string::const_iterator k = bracket_first;

        // Open the regex block.
        regex += "[";

        // A regex range complement uses '^' instead of '!'.
        if(k != bracket_last && *k == '!')
          {
          regex += "^";
          ++k;
          }

        // Convert the remaining characters.
        for(; k != bracket_last; ++k)
          {
          // Backslashes must be escaped.
          if(*k == '\\')
            {
            regex += "\\";
            }

          // Store this character.
          regex += *k;
          }

        // Close the regex block.
        regex += "]";

        // Jump to the end of the bracket string.
        i = bracket_last;
        }
      }
    else
      {
      // A single character matches itself.
      int ch = c;
      if(!(('a' <= ch && ch <= 'z') ||
           ('A' <= ch && ch <= 'Z') ||
           ('0' <= ch && ch <= '9')))
        {
        // Escape the non-alphanumeric character.
        regex += "\\";
        }
#if defined(KWSYS_GLOB_CASE_INDEPENDENT)
      else
        {
        // On case-insensitive systems file names are converted to lower
        // case before matching.
        ch = tolower(ch);
        }
#endif

      // Store the character.
      regex.append(1, static_cast<char>(ch));
      }
    }

  if(require_whole_string)
    {
    regex += "$";
    }
  return regex;
}

//----------------------------------------------------------------------------
void Glob::RecurseDirectory(kwsys_stl::string::size_type start,
  const kwsys_stl::string& dir, bool dir_only)
{
  kwsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    return;
    }
  unsigned long cc;
  kwsys_stl::string fullname;
  kwsys_stl::string realname;
  kwsys_stl::string fname;
  for ( cc = 0; cc < d.GetNumberOfFiles(); cc ++ )
    {
    fname = d.GetFile(cc);
    if ( strcmp(fname.c_str(), ".") == 0 ||
      strcmp(fname.c_str(), "..") == 0  )
      {
      continue;
      }

    if ( start == 0 )
      {
      realname = dir + fname;
      }
    else
      {
      realname = dir + "/" + fname;
      }

#if defined( KWSYS_GLOB_CASE_INDEPENDENT )
    // On Windows and apple, no difference between lower and upper case
    fname = kwsys::SystemTools::LowerCase(fname);
#endif

    if ( start == 0 )
      {
      fullname = dir + fname;
      }
    else
      {
      fullname = dir + "/" + fname;
      }

    if ( !dir_only || !kwsys::SystemTools::FileIsDirectory(realname.c_str()) )
      {
      if ( this->Internals->Expressions[
        this->Internals->Expressions.size()-1].find(fname.c_str()) )
        {
        this->AddFile(this->Internals->Files, realname.c_str());
        }
      }
    if ( kwsys::SystemTools::FileIsDirectory(realname.c_str()) )
      {
      this->RecurseDirectory(start+1, realname, dir_only);
      }
    }
}

//----------------------------------------------------------------------------
void Glob::ProcessDirectory(kwsys_stl::string::size_type start,
  const kwsys_stl::string& dir, bool dir_only)
{
  //kwsys_ios::cout << "ProcessDirectory: " << dir << kwsys_ios::endl;
  bool last = ( start == this->Internals->Expressions.size()-1 );
  if ( last && this->Recurse )
    {
    this->RecurseDirectory(start, dir, dir_only);
    return;
    }
  kwsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    return;
    }
  unsigned long cc;
  kwsys_stl::string fullname;
  kwsys_stl::string realname;
  kwsys_stl::string fname;
  for ( cc = 0; cc < d.GetNumberOfFiles(); cc ++ )
    {
    fname = d.GetFile(cc);
    if ( strcmp(fname.c_str(), ".") == 0 ||
      strcmp(fname.c_str(), "..") == 0  )
      {
      continue;
      }

    if ( start == 0 )
      {
      realname = dir + fname;
      }
    else
      {
      realname = dir + "/" + fname;
      }

#if defined(KWSYS_GLOB_CASE_INDEPENDENT)
    // On case-insensitive file systems convert to lower case for matching.
    fname = kwsys::SystemTools::LowerCase(fname);
#endif

    if ( start == 0 )
      {
      fullname = dir + fname;
      }
    else
      {
      fullname = dir + "/" + fname;
      }

    //kwsys_ios::cout << "Look at file: " << fname << kwsys_ios::endl;
    //kwsys_ios::cout << "Match: "
    // << this->Internals->TextExpressions[start].c_str() << kwsys_ios::endl;
    //kwsys_ios::cout << "Full name: " << fullname << kwsys_ios::endl;

    if ( (!dir_only || !last) &&
      !kwsys::SystemTools::FileIsDirectory(realname.c_str()) )
      {
      continue;
      }

    if ( this->Internals->Expressions[start].find(fname.c_str()) )
      {
      if ( last )
        {
        this->AddFile(this->Internals->Files, realname.c_str());
        }
      else
        {
        this->ProcessDirectory(start+1, realname + "/", dir_only);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool Glob::FindFiles(const kwsys_stl::string& inexpr)
{
  kwsys_stl::string cexpr;
  kwsys_stl::string::size_type cc;
  kwsys_stl::string expr = inexpr;

  this->Internals->Expressions.clear();
  this->Internals->Files.clear();

  if ( !kwsys::SystemTools::FileIsFullPath(expr.c_str()) )
    {
    expr = kwsys::SystemTools::GetCurrentWorkingDirectory();
    expr += "/" + inexpr;
    }
  kwsys_stl::string fexpr = expr;

  int skip = 0;
  int last_slash = 0;
  for ( cc = 0; cc < expr.size(); cc ++ )
    {
    if ( cc > 0 && expr[cc] == '/' && expr[cc-1] != '\\' )
      {
      last_slash = static_cast<int>(cc);
      }
    if ( cc > 0 &&
      (expr[cc] == '[' || expr[cc] == '?' || expr[cc] == '*') &&
      expr[cc-1] != '\\' )
      {
      break;
      }
    }
  if ( last_slash > 0 )
    {
    //kwsys_ios::cout << "I can skip: " << fexpr.substr(0, last_slash)
    //<< kwsys_ios::endl;
    skip = last_slash;
    }
  if ( skip == 0 )
    {
#if defined( KWSYS_GLOB_SUPPORT_NETWORK_PATHS )
    // Handle network paths
    if ( expr[0] == '/' && expr[1] == '/' )
      {
      int cnt = 0;
      for ( cc = 2; cc < expr.size(); cc ++ )
        {
        if ( expr[cc] == '/' )
          {
          cnt ++;
          if ( cnt == 2 )
            {
            break;
            }
          }
        }
      skip = int(cc + 1);
      }
    else
#endif
      // Handle drive letters on Windows
      if ( expr[1] == ':' && expr[0] != '/' )
        {
        skip = 2;
        }
    }

  if ( skip > 0 )
    {
    expr = expr.substr(skip);
    }

  cexpr = "";
  for ( cc = 0; cc < expr.size(); cc ++ )
    {
    int ch = expr[cc];
    if ( ch == '/' )
      {
      if ( cexpr.size() > 0 )
        {
        this->AddExpression(cexpr.c_str());
        }
      cexpr = "";
      }
    else
      {
      cexpr.append(1, static_cast<char>(ch));
      }
    }
  if ( cexpr.size() > 0 )
    {
    this->AddExpression(cexpr.c_str());
    }

  // Handle network paths
  if ( skip > 0 )
    {
    this->ProcessDirectory(0, fexpr.substr(0, skip) + "/",
      true);
    }
  else
    {
    this->ProcessDirectory(0, "/", true);
    }
  return true;
}

//----------------------------------------------------------------------------
void Glob::AddExpression(const char* expr)
{
  this->Internals->Expressions.push_back(
    kwsys::RegularExpression(
      this->PatternToRegex(expr).c_str()));
}

//----------------------------------------------------------------------------
void Glob::SetRelative(const char* dir)
{
  if ( !dir )
    {
    this->Relative = "";
    return;
    }
  this->Relative = dir;
}

//----------------------------------------------------------------------------
const char* Glob::GetRelative()
{
  if ( this->Relative.empty() )
    {
    return 0;
    }
  return this->Relative.c_str();
}

//----------------------------------------------------------------------------
void Glob::AddFile(kwsys_stl::vector<kwsys_stl::string>& files, const char* file)
{
  if ( !this->Relative.empty() )
    {
    files.push_back(kwsys::SystemTools::RelativePath(this->Relative.c_str(), file));
    }
  else
    {
    files.push_back(file);
    }
}

} // namespace KWSYS_NAMESPACE

