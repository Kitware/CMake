/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
  BuildRules::iterator s = m_BuildRules.find(cmd.GetSourceName());
  if(s == m_BuildRules.end())
    {
    // The source was not found.  Add it with this command.
    m_BuildRules[cmd.GetSourceName()][cmd.GetCommand()].
      m_Depends.insert(cmd.GetDepends().begin(),cmd.GetDepends().end());
    m_BuildRules[cmd.GetSourceName()][cmd.GetCommand()].
      m_Outputs.insert(cmd.GetOutputs().begin(),cmd.GetOutputs().end());
    return;
    }
  
  // The source already exists.  See if the command exists.
  Commands& commands = s->second;
  Commands::iterator c = commands.find(cmd.GetCommand());
  if(c == commands.end())
    {
    // The command did not exist.  Add it.
    commands[cmd.GetCommand()].m_Depends.insert(cmd.GetDepends().begin(), cmd.GetDepends().end());
    commands[cmd.GetCommand()].m_Outputs.insert(cmd.GetOutputs().begin(), cmd.GetOutputs().end());
    return;
    }
  
  // The command already exists for this source.  Merge the sets.
  CommandFiles& commandFiles = c->second;
  commandFiles.m_Depends.insert(cmd.GetDepends().begin(), cmd.GetDepends().end());
  commandFiles.m_Outputs.insert(cmd.GetOutputs().begin(), cmd.GetOutputs().end());
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
      std::cout << "Command: " << j->first.c_str() << "\n";
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



