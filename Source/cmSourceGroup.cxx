/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

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
  m_BuildRules(r.m_BuildRules)
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
void cmSourceGroup::AddSource(const char* name)
{
  BuildRules::iterator s = m_BuildRules.find(name);
  if(s == m_BuildRules.end())
    {
    // The source was not found.  Add it with no commands.
    m_BuildRules[name];
    return;
    }
}


/**
 * Add a source and corresponding custom command to the group.  If the
 * source already exists, the command will be added to its set of commands.
 * If the command also already exists, the given dependencies and outputs
 * are added to it.
 */
void cmSourceGroup::AddCustomCommand(const cmCustomCommand &cmd)
{
  std::string commandAndArgs = cmd.GetCommandAndArguments();
  BuildRules::iterator s = m_BuildRules.find(cmd.GetSourceName());
  if(s == m_BuildRules.end())
    {
    // The source was not found.  Add it with this command.
    CommandFiles& cmdFiles = m_BuildRules[cmd.GetSourceName()][commandAndArgs];
    cmdFiles.m_Command = cmd.GetCommand();
    cmdFiles.m_Arguments = cmd.GetArguments();
    cmdFiles.m_Depends.insert(cmd.GetDepends().begin(),cmd.GetDepends().end());
    cmdFiles.m_Outputs.insert(cmd.GetOutputs().begin(),cmd.GetOutputs().end());
    return;
    }
  
  // The source already exists.  See if the command exists.
  Commands& commands = s->second;
  Commands::iterator c = commands.find(commandAndArgs);
  if(c == commands.end())
    {
    // The command did not exist.  Add it.
    commands[commandAndArgs].m_Command = cmd.GetCommand();
    commands[commandAndArgs].m_Arguments = cmd.GetArguments();
    commands[commandAndArgs].m_Depends.insert(cmd.GetDepends().begin(),
                                              cmd.GetDepends().end());
    commands[commandAndArgs].m_Outputs.insert(cmd.GetOutputs().begin(),
                                              cmd.GetOutputs().end());
    return;
    }
  
  // The command already exists for this source.  Merge the sets.
  CommandFiles& commandFiles = c->second;
  commandFiles.m_Depends.insert(cmd.GetDepends().begin(), 
                                cmd.GetDepends().end());
  commandFiles.m_Outputs.insert(cmd.GetOutputs().begin(),
                                cmd.GetOutputs().end());
}

void cmSourceGroup::Print() const
{
  std::cout << "cmSourceGroup: " << m_Name.c_str() << "\n";
  for(BuildRules::const_iterator i = m_BuildRules.begin();
      i != m_BuildRules.end(); ++i)
    {
    std::cout << "BuildRule: " << i->first.c_str() << "\n";
    for(Commands::const_iterator j = i->second.begin();
        j != i->second.end(); ++j)
      {
      std::cout << "FullCommand: " << j->first.c_str() << "\n";
      std::cout << "Command: " << j->second.m_Command.c_str() << "\n";
      std::cout << "Arguments: " << j->second.m_Arguments.c_str() << "\n";
      std::cout << "Command Outputs " << j->second.m_Outputs.size() << "\n";
      std::cout << "Command Depends " << j->second.m_Depends.size() << "\n";
      }
    }
}


void cmSourceGroup::CommandFiles::Merge(const CommandFiles &r)
{
  std::set<std::string>::const_iterator dep = r.m_Depends.begin();
  std::set<std::string>::const_iterator out = r.m_Outputs.begin();
  for (;dep != r.m_Depends.end(); ++dep)
    {
    this->m_Depends.insert(*dep);
    }
  for (;out != r.m_Outputs.end(); ++out)
    {
    this->m_Outputs.insert(*out);
    }
}



