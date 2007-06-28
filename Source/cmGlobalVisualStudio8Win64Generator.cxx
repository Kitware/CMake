/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio8Win64Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"



cmGlobalVisualStudio8Win64Generator::cmGlobalVisualStudio8Win64Generator()
{
  this->PlatformName = "x64";
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio8Win64Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion8();
  lg->SetPlatformName(this->PlatformName.c_str());
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Win64Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Visual Studio .NET 2005 Win64 project files.";
  entry.full = "";
}

void cmGlobalVisualStudio8Win64Generator
::EnableLanguage(std::vector<std::string>const &  lang, 
                 cmMakefile *mf, bool optional)
{
  mf->AddDefinition("CMAKE_FORCE_WIN64", "TRUE");
  cmGlobalVisualStudio8Generator::EnableLanguage(lang, mf, optional);
}
