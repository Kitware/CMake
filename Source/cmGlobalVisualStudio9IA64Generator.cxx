/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio9IA64Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"


cmGlobalVisualStudio9IA64Generator::cmGlobalVisualStudio9IA64Generator()
{
  this->ArchitectureId = "Itanium";
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio9IA64Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg =
    new cmLocalVisualStudio7Generator(cmLocalVisualStudioGenerator::VS9);
  lg->SetPlatformName(this->GetPlatformName());
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9IA64Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio 9 2008 Itanium project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9IA64Generator
::AddPlatformDefinitions(cmMakefile* mf)
{
  cmGlobalVisualStudio9Generator::AddPlatformDefinitions(mf);
  mf->AddDefinition("CMAKE_FORCE_IA64", "TRUE");
}
