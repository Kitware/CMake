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
#include <cmsys/RegularExpression.hxx>

class cmSourceFile;

/** \class cmSourceGroup
 * \brief Hold a group of sources as specified by a SOURCE_GROUP command.
 *
 * cmSourceGroup holds a regular expression and a list of files.  When
 * local generators are about to generate the rules for a target's
 * files, the set of source groups is consulted to group files
 * together.  A file is placed into the last source group that lists
 * the file by name.  If no group lists the file, it is placed into
 * the last group whose regex matches it.
 */
class cmSourceGroup
{
public:
  cmSourceGroup(const char* name, const char* regex);
  ~cmSourceGroup() {}
  
  /**
   * Set the regular expression for this group.
   */
  void SetGroupRegex(const char* regex);
  
  /**
   * Add a file name to the explicit list of files for this group.
   */
  void AddGroupFile(const char* name);
  
  /**
   * Get the name of this group.
   */
  const char* GetName() const;
  
  /**
   * Check if the given name matches this group's regex.
   */
  bool MatchesRegex(const char* name);
  
  /**
   * Check if the given name matches this group's explicit file list.
   */
  bool MatchesFiles(const char* name);
  
  /**  
   * Assign the given source file to this group.  Used only by
   * generators.
   */
  void AssignSource(const cmSourceFile* sf);

  /**
   * Get the list of the source files that have been assigned to this
   * source group.
   */
  const std::vector<const cmSourceFile*>& GetSourceFiles() const;
  std::vector<const cmSourceFile*>& GetSourceFiles();
  
private:
  /**
   * The name of the source group.
   */
  std::string m_Name;
  
  /**
   * The regular expression matching the files in the group.
   */
  cmsys::RegularExpression m_GroupRegex;
  
  /**
   * Set of file names explicitly added to this group.
   */
  std::set<cmStdString> m_GroupFiles;
  
  /**
   * Vector of all source files that have been assigned to
   * this group.
   */
  std::vector<const cmSourceFile*> m_SourceFiles;
};

#endif
