/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio10Win64Generator.h"
#include "cmMakefile.h"
#include "cmake.h"

//----------------------------------------------------------------------------
cmGlobalVisualStudio10Win64Generator::cmGlobalVisualStudio10Win64Generator()
{
  this->PlatformName = "x64";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Win64Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio 10 Win64 project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Win64Generator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile *mf, bool optional)
{
  mf->AddDefinition("CMAKE_FORCE_WIN64", "TRUE");
  cmGlobalVisualStudio10Generator::EnableLanguage(lang, mf, optional);
}
