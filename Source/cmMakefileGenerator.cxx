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
#include "cmMakefileGenerator.h"

// static list of registered generators
std::map<cmStdString, cmMakefileGenerator*>
cmMakefileGenerator::s_RegisteredGenerators;


void cmMakefileGenerator::SetMakefile(cmMakefile* mf)
{
  m_Makefile = mf;
}

void cmMakefileGenerator::UnRegisterGenerators()
{
   for(std::map<cmStdString, cmMakefileGenerator*>::iterator i
        = s_RegisteredGenerators.begin(); 
      i != s_RegisteredGenerators.end(); ++i)
    {
    delete i->second;
    }
   s_RegisteredGenerators = std::map<cmStdString, cmMakefileGenerator*>();
}

void cmMakefileGenerator::GetRegisteredGenerators(std::vector<std::string>& names)
{
  for(std::map<cmStdString, cmMakefileGenerator*>::iterator i
        = s_RegisteredGenerators.begin(); 
      i != s_RegisteredGenerators.end(); ++i)
    {
    names.push_back(i->first);
    }
}


void 
cmMakefileGenerator::RegisterGenerator(cmMakefileGenerator* mg)
{
  std::map<cmStdString, cmMakefileGenerator*>::iterator i = 
    s_RegisteredGenerators.find(mg->GetName());
  // delete re-registered objects
  if(i != s_RegisteredGenerators.end())
    {
    delete i->second;
    }
  s_RegisteredGenerators[mg->GetName()] = mg;
}


cmMakefileGenerator* 
cmMakefileGenerator::CreateGenerator(const char* name)
{
  std::map<cmStdString, cmMakefileGenerator*>::iterator i;
  for(i = s_RegisteredGenerators.begin();
      i != s_RegisteredGenerators.end(); ++i)
    {
    cmMakefileGenerator* gen = i->second;
    if(strcmp(name, gen->GetName()) == 0)
      {
      return gen->CreateObject();
      }
    }
  return 0;
}
