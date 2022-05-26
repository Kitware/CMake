/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalMinGWMakefileGenerator.h"

#include "cmDocumentationEntry.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmGlobalMinGWMakefileGenerator::cmGlobalMinGWMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeMinGWFindMake.cmake";
  this->ForceUnixPaths = true;
  this->ToolSupportsColor = true;
  this->UseLinkScript = true;
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetMinGWMake(true);
}

void cmGlobalMinGWMakefileGenerator::GetDocumentation(
  cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalMinGWMakefileGenerator::GetActualName();
  entry.Brief = "Generates a make file for use with mingw32-make.";
}
