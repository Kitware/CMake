/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio11ARMGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

//----------------------------------------------------------------------------
void cmGlobalVisualStudio11ARMGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio 11 ARM project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio11ARMGenerator
::AddPlatformDefinitions(cmMakefile* mf)
{
  this->cmGlobalVisualStudio11Generator::AddPlatformDefinitions(mf);
  mf->AddDefinition("MSVC_C_ARCHITECTURE_ID", "ARM");
  mf->AddDefinition("MSVC_CXX_ARCHITECTURE_ID", "ARM");
}
