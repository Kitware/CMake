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


/**
 * The constructor initializes the group's regular expression.
 */
cmSourceGroup::cmSourceGroup(const char* name, const char* regex):
  m_Name(name),
  m_GroupRegex(regex)
{
}


/**
 * Copy constructor.
 */
cmSourceGroup::cmSourceGroup(const cmSourceGroup& r):
  m_Name(r.m_Name),
  m_GroupRegex(r.m_GroupRegex),
  m_SourceFiles(r.m_SourceFiles)
{
}


/**
 * Returns whether the given name matches the group's regular expression.
 */
bool cmSourceGroup::Matches(const char* name)
{
  return m_GroupRegex.find(name);
}


/**
 * Add a source to the group that the compiler will know how to build.
 */
void cmSourceGroup::AddSource(const char* /* name */, const cmSourceFile* sf)
{
  m_SourceFiles.push_back(sf);
}

