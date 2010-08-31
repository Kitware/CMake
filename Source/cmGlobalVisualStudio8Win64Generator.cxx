/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio8Win64Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"



cmGlobalVisualStudio8Win64Generator::cmGlobalVisualStudio8Win64Generator()
{
  this->ArchitectureId = "x64";
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio8Win64Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion8();
  lg->SetPlatformName(this->GetPlatformName());
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Win64Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio .NET 2005 Win64 project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Win64Generator
::AddPlatformDefinitions(cmMakefile* mf)
{
  this->cmGlobalVisualStudio8Generator::AddPlatformDefinitions(mf);
  mf->AddDefinition("CMAKE_FORCE_WIN64", "TRUE");
}
