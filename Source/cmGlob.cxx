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
  m_Recurse = false;
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
      if ( j < n && ( expr[j] == '!' || expr[j] == '^' ) )
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
        if ( stuff[0] == '!' || stuff[0] == '^' )
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

void cmGlob::RecurseDirectory(const std::string& dir, bool dir_only)
{
  cmsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    return;
    }
  unsigned long cc;
  std::string fullname;
  for ( cc = 0; cc < d.GetNumberOfFiles(); cc ++ )
    {
    if ( strcmp(d.GetFile(cc), ".") == 0 ||
      strcmp(d.GetFile(cc), "..") == 0  )
      {
      continue;
      }
    fullname = dir + "/" + d.GetFile(cc);
    if ( !dir_only || !cmsys::SystemTools::FileIsDirectory(fullname.c_str()) )
      {
      if ( m_Internals->Expressions[m_Internals->Expressions.size()-1].find(d.GetFile(cc)) )
        {
        m_Internals->Files.push_back(fullname);
        }
      }
    if ( cmsys::SystemTools::FileIsDirectory(fullname.c_str()) )
      {
      this->RecurseDirectory(fullname, dir_only);
      }
    }
}

void cmGlob::ProcessDirectory(std::string::size_type start, 
  const std::string& dir, bool dir_only)
{
  bool last = ( start == m_Internals->Expressions.size()-1 );
  if ( last && m_Recurse )
    {
    this->RecurseDirectory(dir, dir_only);
    return;
    }
  cmsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    return;
    }
  unsigned long cc;
  std::string fullname;
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
      continue;
      }

    if ( m_Internals->Expressions[start].find(d.GetFile(cc)) )
      {
      if ( last )
        {
        m_Internals->Files.push_back(fullname);
        }
      else
        {
        this->ProcessDirectory(start+1, fullname, dir_only);
        }
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
  if ( expr[1] == ':' && expr[0] != '/' )
    {
    expr = expr.substr(2);
    }
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
  if ( inexpr[1] == ':' && inexpr[0] != '/' )
    {
    std::string startdir = "A:/";
    if ( inexpr[0] >= 'a' && inexpr[0] <= 'z' ||
      inexpr[0] >= 'A' && inexpr[0] <= 'Z')
      {
      startdir[0] = inexpr[0];
      this->ProcessDirectory(0, startdir, true);
      }
    else 
      {
      return false;
      }
    }
  else
    {
    this->ProcessDirectory(0, "/", true);
    }
  return true;
}

void cmGlob::AddExpression(const char* expr)
{
  m_Internals->Expressions.push_back(
    cmsys::RegularExpression(
      this->ConvertExpression(expr).c_str()));
}

