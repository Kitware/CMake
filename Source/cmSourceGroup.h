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
#ifndef cmSourceGroup_h
#define cmSourceGroup_h

#include "cmStandardIncludes.h"
#include "cmRegularExpression.h"
class cmSourceFile;

/** \class cmSourceGroup
 * \brief Hold a group of sources as specified by a SOURCE_GROUP command.
 *
 * cmSourceGroup holds all the source files and corresponding commands
 * for files matching the regular expression specified for the group.
 */
class cmSourceGroup
{
public:
  cmSourceGroup(const char* name, const char* regex);
  cmSourceGroup(const cmSourceGroup&);
  ~cmSourceGroup() {}
  
  void SetGroupRegex(const char* regex)
    { m_GroupRegex.compile(regex); }
  void AddSource(const char* name, const cmSourceFile*);
  const char* GetName() const
    { return m_Name.c_str(); }
  bool Matches(const char *);

  /**
   * Get the list of the source files used by this target
   */
  const std::vector<const cmSourceFile*> &GetSourceFiles() const 
    {return m_SourceFiles;}
  std::vector<const cmSourceFile*> &GetSourceFiles() {return m_SourceFiles;}
  
private:
  /**
   * The name of the source group.
   */
  std::string m_Name;
  
  /**
   * The regular expression matching the files in the group.
   */
  cmRegularExpression m_GroupRegex;
  
  /**
   * vector of all source files in this source group
   */
  std::vector<const cmSourceFile*> m_SourceFiles;
};

#endif
