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
#include "cmInstallFilesCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmInstallFilesCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Create an INSTALL_FILES target specifically for this path.
  m_TargetName = "INSTALL_FILES_"+args[0];
  cmTarget target;
  target.SetInAll(false);
  target.SetType(cmTarget::INSTALL_FILES);
  target.SetInstallPath(args[0].c_str());
  m_Makefile->GetTargets().insert(cmTargets::value_type(m_TargetName, target));

  std::vector<std::string>::iterator s = args.begin();
  for (++s;s != args.end(); ++s)
    {
    m_FinalArgs.push_back(*s);
    }
  
  return true;
}

void cmInstallFilesCommand::FinalPass() 
{
  std::string testf;
  std::string ext = m_FinalArgs[0];
  std::vector<std::string>& targetSourceLists =
    m_Makefile->GetTargets()[m_TargetName].GetSourceLists();
  
  // two different options
  if (m_FinalArgs.size() > 1)
    {
    // now put the files into the list
    std::vector<std::string>::iterator s = m_FinalArgs.begin();
    ++s;
    // for each argument, get the files 
    for (;s != m_FinalArgs.end(); ++s)
      {
      // replace any variables
      std::string temps = *s;
      m_Makefile->ExpandVariablesInString(temps);
      // look for a srclist
      if (m_Makefile->GetSources().find(temps) != m_Makefile->GetSources().end())
        {
        const std::vector<cmSourceFile> &clsList = 
          m_Makefile->GetSources().find(temps)->second;
        std::vector<cmSourceFile>::const_iterator c = clsList.begin();
        for (; c != clsList.end(); ++c)
          {
          testf = c->GetSourceName() + ext;
          // add to the result
          targetSourceLists.push_back(testf);
          }
        }
      // if one wasn't found then assume it is a single class
      else
        {
        testf = temps + ext;
        // add to the result
        targetSourceLists.push_back(testf);
        }
      }
    }
  else     // reg exp list
    {
    std::vector<std::string> files;
    std::string regex = m_FinalArgs[0].c_str();
    m_Makefile->ExpandVariablesInString(regex);
    cmSystemTools::Glob(m_Makefile->GetCurrentDirectory(),
                        regex.c_str(), files);
    
    std::vector<std::string>::iterator s = files.begin();
    // for each argument, get the files 
    for (;s != files.end(); ++s)
      {
      targetSourceLists.push_back(*s);
      }
    }
}

      
