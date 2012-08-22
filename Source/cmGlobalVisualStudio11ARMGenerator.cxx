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
cmGlobalVisualStudio11ARMGenerator::cmGlobalVisualStudio11ARMGenerator()
{
  this->ArchitectureId = "ARM";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio11ARMGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio 11 ARM project files.";
  entry.Full = "";
}
