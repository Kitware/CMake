/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlob.h"

#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/SystemTools.hxx>

#include <stdio.h>

class cmGlobInternal
{
public:
  std::vector<std::string> Files;
  std::vector<cmsys::RegularExpression> Expressions;
};

cmGlob::cmGlob()
{
  m_Internals = new cmGlobInternal;
}

cmGlob::~cmGlob()
{
  delete m_Internals;
}

void cmGlob::Escape(int ch, char* buffer)
{
  if (! (
      'a' <= ch && ch <= 'z' || 
      'A' <= ch && ch <= 'Z' || 
      '0' <= ch && ch <= '9') )
    {
    sprintf(buffer, "\\%c", ch);
    }
  else
    {
    sprintf(buffer, "%c", ch);
    }
}

std::vector<std::string>& cmGlob::GetFiles()
{
  return m_Internals->Files;
}

std::string cmGlob::ConvertExpression(const std::string& expr)
{
  
  std::string::size_type i = 0;
  std::string::size_type n = expr.size();

  std::string res = "^";
  std::string stuff = "";

  while ( i < n )
    {
    int c = expr[i];
    i = i+1;
    if ( c == '*' )
      {
      res = res + ".*";
      }
    else if ( c == '?' )
      {
      res = res + ".";
      }
    else if ( c == '[' )
      {
      std::string::size_type j = i;
      if ( j < n && expr[j] == '!' )
        {
        j = j+1;
        }
      if ( j < n && expr[j] == ']' )
        {
        j = j+1;
        } 
      while ( j < n && expr[j] != ']' )
        {
        j = j+1;
        }
      if ( j >= n )
        {
        res = res + "\\[";
        }
      else
        {
        stuff = "";
        std::string::size_type cc;
        for ( cc = i; cc < j; cc ++ )
          {
          if ( expr[cc] == '\\' )
            {
            stuff += "\\\\";
            }
          else
            {
            stuff += expr[cc];
            }
          }
        i = j+1;
        if ( stuff[0] == '!' )
          {
          stuff = '^' + stuff.substr(1);
          }
        else if ( stuff[0] == '^' )
          {
          stuff = '\\' + stuff;
          }
        res = res + "[" + stuff + "]";
        }
      }
    else
      {
      char buffer[100];
      buffer[0] = 0;
      this->Escape(c, buffer);
      res = res + buffer;
      }
    }
  return res + "$";
}

void cmGlob::ProcessDirectory(std::string::size_type start, 
  const std::string& dir, bool dir_only)
{
  cmsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    //std::cout << "Cannot open directory: " << dir.c_str() << std::endl;
    return;
    }
  unsigned long cc;
  std::string fullname;
  bool last = ( start == m_Internals->Expressions.size()-1 );
  //std::cout << "Last: " << last << " Dironly: " << dir_only << std::endl;
  for ( cc = 0; cc < d.GetNumberOfFiles(); cc ++ )
    {
    if ( strcmp(d.GetFile(cc), ".") == 0 ||
     strcmp(d.GetFile(cc), "..") == 0  )
      {
      continue;
      }
    if ( start == 0 )
      {
      fullname = dir + d.GetFile(cc);
      }
    else
      {
      fullname = dir + "/" + d.GetFile(cc);
      }

    if ( (!dir_only || !last) && !cmsys::SystemTools::FileIsDirectory(fullname.c_str()) )
      {
      //std::cout << " Ignore: " << fullname.c_str() << std::endl;
      continue;
      }
    if ( m_Internals->Expressions[start].find(d.GetFile(cc)) )
      {
      //std::cout << " Matches: " << fullname.c_str() << std::endl;
      if ( last )
        {
        //std::cout << "--- find file: " << fullname.c_str() << "---" << std::endl;
        m_Internals->Files.push_back(fullname);
        }
      else
        {
        this->ProcessDirectory(start+1, fullname, dir_only);
        }
      }
    else
      {
      //std::cout << " Not Matches: " << fullname.c_str() << std::endl;
      }
    }
}

bool cmGlob::FindFiles(const std::string& inexpr)
{
  std::string cexpr;
  std::string::size_type cc;
  std::string expr = inexpr;

  m_Internals->Expressions.empty();
  m_Internals->Files.empty();

  if ( !cmsys::SystemTools::FileIsFullPath(expr.c_str()) )
    {
    expr = cmsys::SystemTools::GetCurrentWorkingDirectory();
    expr += "/" + inexpr;
    }
  std::cout << "Expr: " << expr << std::endl;
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
      cexpr.append(1, (char)ch);
      }
    }
  if ( cexpr.size() > 0 )
    {
    this->AddExpression(cexpr.c_str());
    }

  this->ProcessDirectory(0, "/", true);
  return true;
}

void cmGlob::AddExpression(const char* expr)
{
  m_Internals->Expressions.push_back(
    cmsys::RegularExpression(
      this->ConvertExpression(expr).c_str()));
}

