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
#include "cmCableClassSetCommand.h"
#include "cmCacheManager.h"
#include "cmTarget.h"

// cmCableClassSetCommand
bool cmCableClassSetCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // First, we want to expand all CMAKE variables in all arguments.
  for(std::vector<std::string>::iterator a = args.begin();
      a != args.end(); ++a)
    {
    m_Makefile->ExpandVariablesInString(*a);
    }
  
  // The first argument is the name of the set.
  std::vector<std::string>::const_iterator arg = args.begin();
  m_ClassSetName = *arg++;
  
  // Create the new class set.
  cmCableClassSet* classSet = new cmCableClassSet(m_ClassSetName.c_str());
  
  // Add all the regular entries.
  for(; (arg != args.end()) && (*arg != "SOURCES_BEGIN"); ++arg)
    {
    classSet->ParseAndAddElement(arg->c_str(), m_Makefile);
    }
  
  // Add any sources that are associated with all the members.
  if(arg != args.end())
    {
    for(++arg; arg != args.end(); ++arg)
      {
      classSet->AddSource(arg->c_str());
      }
    }
  
  // Store the class set in the makefile.
  m_Makefile->RegisterData(classSet);
  
  return true;
}

