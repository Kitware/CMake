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
#include "cmSourceGroup.h"

//----------------------------------------------------------------------------
cmSourceGroup::cmSourceGroup(const char* name, const char* regex): m_Name(name)
{
  this->SetGroupRegex(regex);
}

//----------------------------------------------------------------------------
void cmSourceGroup::SetGroupRegex(const char* regex)
{
  if(regex)
    {
    m_GroupRegex.compile(regex);
    }
  else
    {
    m_GroupRegex.compile("^$");
    }
}
  
//----------------------------------------------------------------------------
void cmSourceGroup::AddGroupFile(const char* name)
{
  m_GroupFiles.insert(name);
}
  
//----------------------------------------------------------------------------
const char* cmSourceGroup::GetName() const
{
  return m_Name.c_str();
}
  
//----------------------------------------------------------------------------
bool cmSourceGroup::MatchesRegex(const char* name)
{
  return m_GroupRegex.find(name);
}

//----------------------------------------------------------------------------
bool cmSourceGroup::MatchesFiles(const char* name)
{
  std::set<cmStdString>::const_iterator i = m_GroupFiles.find(name);
  if(i != m_GroupFiles.end())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmSourceGroup::AssignSource(const cmSourceFile* sf)
{
  m_SourceFiles.push_back(sf);
}

//----------------------------------------------------------------------------
const std::vector<const cmSourceFile*>& cmSourceGroup::GetSourceFiles() const
{
  return m_SourceFiles;
}

//----------------------------------------------------------------------------
std::vector<const cmSourceFile*>& cmSourceGroup::GetSourceFiles()
{
  return m_SourceFiles;
}
