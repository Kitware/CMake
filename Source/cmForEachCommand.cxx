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
#include "cmForEachCommand.h"
#include "cmCacheManager.h"

bool cmForEachFunctionBlocker::
IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                  cmMakefile &mf) 
{
  
  // at end of for each execute recorded commands
  if (!strcmp(name,"ENDFOREACH") && args[0] == m_Args[0])
    {
    std::string variable = "${";
    variable += m_Args[0];
    variable += "}"; 
    std::vector<std::string>::const_iterator j = m_Args.begin();
    ++j;
    
    for( ; j != m_Args.end(); ++j)
      {   
      // perform string replace
	for(unsigned int c = 0; c < m_Commands.size(); ++c)
        {
        std::vector<std::string> newArgs;
        for (std::vector<std::string>::const_iterator k = 
               m_CommandArguments[c].begin();
             k != m_CommandArguments[c].end(); ++k)
          {
          std::string tmps = *k;
          cmSystemTools::ReplaceString(tmps, variable.c_str(),
                                       j->c_str());
          newArgs.push_back(tmps);
          }
        // execute command
        mf.ExecuteCommand(m_Commands[c],newArgs);
        }
      }
    return false;
    }

  // record the command
  m_Commands.push_back(name);
  std::vector<std::string> newArgs;
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    newArgs.push_back(*j);
    }
  m_CommandArguments.push_back(newArgs);
  
  // always return true
  return true;
}

bool cmForEachFunctionBlocker::
ShouldRemove(const char *name, const std::vector<std::string> &args, 
             cmMakefile &mf) 
{
  if (!strcmp(name,"ENDFOREACH") && args[0] == m_Args[0])
    {
    return true;
    }
  return false;
}

void cmForEachFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  cmSystemTools::Error("The end of a CMakeLists file was reached with a FOREACH statement that was not closed properly. Within the directory: ", 
                       mf.GetCurrentDirectory());
}

bool cmForEachCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmForEachFunctionBlocker *f = new cmForEachFunctionBlocker();
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    f->m_Args.push_back(*j);
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

