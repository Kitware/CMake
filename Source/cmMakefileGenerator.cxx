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
#include "cmMakefileGenerator.h"

// static list of registered generators
std::map<cmStdString, cmMakefileGenerator*>
cmMakefileGenerator::s_RegisteredGenerators;
std::map<cmStdString, bool> cmMakefileGenerator::s_LanguageEnabled;


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



void cmMakefileGenerator::SetLanguageEnabled(const char* l)
{
  s_LanguageEnabled[l] = true;
}



bool cmMakefileGenerator::GetLanguageEnabled(const char* l)
{
  return (s_LanguageEnabled.count(l) > 0);
}



void cmMakefileGenerator::ClearEnabledLanguages()
{
  s_LanguageEnabled.clear();
}

