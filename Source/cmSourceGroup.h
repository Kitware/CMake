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
#include "cmCustomCommand.h"
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
  
  struct CommandFiles
  {
    CommandFiles() {}
    CommandFiles(const CommandFiles& r):
      m_Outputs(r.m_Outputs), m_Depends(r.m_Depends) {}
    
    void Merge(const CommandFiles &r);
    
    std::string m_Command;
    std::string m_Arguments;
    std::set<std::string> m_Outputs;
    std::set<std::string> m_Depends;
  };
  
  /**
   * Map from command to its output/depends sets.
   */
  typedef std::map<cmStdString, CommandFiles> Commands;

  struct SourceAndCommands
  {
    SourceAndCommands(): m_SourceFile(0) {}
    const cmSourceFile* m_SourceFile;
    Commands m_Commands;
  };
  /**
   * Map from source to command map.
   */
  typedef std::map<cmStdString, SourceAndCommands>  BuildRules;

  bool Matches(const char* name);
  void SetGroupRegex(const char* regex)
    { m_GroupRegex.compile(regex); }
  void AddSource(const char* name, const cmSourceFile*);
  void AddCustomCommand(const cmCustomCommand &cmd);
  const char* GetName() const
    { return m_Name.c_str(); }
  const BuildRules& GetBuildRules() const
    { return m_BuildRules; }
  void Print() const;
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
   * Map from source name to the commands to build from the source.
   * Some commands may build from files that the compiler also knows how to
   * build.
   */
  BuildRules m_BuildRules;  
};

#endif
