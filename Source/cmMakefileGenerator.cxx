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
std::map<cmStdString, bool> cmMakefileGenerator::s_LanguageEnabled;


void cmMakefileGenerator::SetMakefile(cmMakefile* mf)
{
  m_Makefile = mf;
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

