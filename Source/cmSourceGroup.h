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
#ifndef cmSourceGroup_h
#define cmSourceGroup_h

#include "cmStandardIncludes.h"
#include "cmRegularExpression.h"
#include "cmCustomCommand.h"


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
  typedef std::map<std::string, Commands>  BuildRules;

  bool Matches(const char* name);
  void SetGroupRegex(const char* regex)
    { m_GroupRegex.compile(regex); }
  void AddSource(const char* name);
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
