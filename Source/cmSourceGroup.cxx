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
  m_Sources(r.m_Sources),
  m_CustomCommands(r.m_CustomCommands)
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
 * Add a source and corresponding custom command to the group.  If the
 * source already exists, the command will be added to its set of commands.
 * If the command also already exists, the given dependencies and outputs
 * are added to it.
 */
void cmSourceGroup::AddCustomCommand(const char* source,
                                     const char* command,
                                     const std::vector<std::string>& depends,
                                     const std::vector<std::string>& outputs) 
{
  CustomCommands::iterator s = m_CustomCommands.find(source);
  if(s == m_CustomCommands.end())
    {
    // The source was not found.  Add it with this command.
    m_CustomCommands[source][command].m_Depends.insert(depends.begin(),
                                                       depends.end());
    m_CustomCommands[source][command].m_Outputs.insert(outputs.begin(),
                                                       outputs.end());
    return;
    }
  
  // The source already exists.  See if the command exists.
  Commands& commands = s->second;
  Commands::iterator c = commands.find(command);
  if(c == commands.end())
    {
    // The command did not exist.  Add it.
    commands[command].m_Depends.insert(depends.begin(), depends.end());
    commands[command].m_Outputs.insert(outputs.begin(), outputs.end());
    return;
    }
  
  // The command already exists for this source.  Merge the sets.
  CommandFiles& commandFiles = c->second;
  commandFiles.m_Depends.insert(depends.begin(), depends.end());
  commandFiles.m_Outputs.insert(outputs.begin(), outputs.end());
}
