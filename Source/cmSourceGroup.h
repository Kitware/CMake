/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmSourceGroup_h
#define cmSourceGroup_h

#include "cmStandardIncludes.h"
#include "cmRegularExpression.h"
#include <set>

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
    
    std::set<std::string> m_Outputs;
    std::set<std::string> m_Depends;
  };
  
  /**
   * Map from command to its output/depends sets.
   */
  typedef std::map<std::string, CommandFiles> Commands;

  /**
   * Map from source to command map.
   */
  typedef std::map<std::string, Commands>  CustomCommands;

  bool Matches(const char* name);
  void SetGroupRegex(const char* regex)
    { m_GroupRegex.compile(regex); }
  void AddSource(const char* name)
    { m_Sources.push_back(name); }
  void AddCustomCommand(const char* source,
                        const char* command,
                        const std::vector<std::string>& depends,
                        const std::vector<std::string>& outputs);
  const char* GetName() const
    { return m_Name.c_str(); }
  const std::vector<std::string>& GetSources() const
    { return m_Sources; }
  const CustomCommands& GetCustomCommands() const
    { return m_CustomCommands; }
  
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
   * The sources in this group that the compiler will know how to build.
   */
  std::vector<std::string> m_Sources;
  
  /**
   * The custom commands in this group and their corresponding sources.
   */
  CustomCommands m_CustomCommands;  
};

#endif
