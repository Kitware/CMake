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
#include "cmVariableRequiresCommand.h"
#include "cmCacheManager.h"

// cmLibraryCommand
bool cmVariableRequiresCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  m_Arguments = args;
  return true;
}

void cmVariableRequiresCommand::FinalPass()
{
  std::string testVarible = m_Arguments[0]; 
  if(!m_Makefile->IsOn(testVarible.c_str()))
    {
    return;
    }
  std::string resultVarible = m_Arguments[1];
  bool requirementsMet = true;
  std::string notSet;
  bool hasAdvanced = false;
  for(unsigned int i = 2; i < m_Arguments.size(); ++i)
    {
    if(!m_Makefile->IsOn(m_Arguments[i].c_str()))
      {
      requirementsMet = false;
      notSet += m_Arguments[i];
      notSet += "\n";
      if(cmCacheManager::GetInstance()->IsAdvanced(m_Arguments[i].c_str()))
        {
        hasAdvanced = true;
        }
      }
    }
  const char* reqVar = m_Makefile->GetDefinition(resultVarible.c_str());
  // if reqVar is unset, then set it to requirementsMet 
  // if reqVar is set to true, but requirementsMet is false , then
  // set reqVar to false.
  if(!reqVar || (!requirementsMet && m_Makefile->IsOn(reqVar)))
    {
    m_Makefile->AddDefinition(resultVarible.c_str(), requirementsMet);
    }

  if(!requirementsMet)
    {
    std::string message = "Variable assertion failed:\n";
    message += testVarible + " Requires that the following unset varibles are set:\n";
    message += notSet;
    message += "\nPlease set them, or set ";
    message += testVarible + " to false, and re-configure.\n";
    if(hasAdvanced)
      {
      message += "One or more of the required variables is advanced.  To set the variable, you must turn on advanced mode in cmake.";
      }
    cmSystemTools::Error(message.c_str());
    }
}
