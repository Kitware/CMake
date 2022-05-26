/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalMSYSMakefileGenerator.h"

#include "cmsys/FStream.hxx"

#include "cmDocumentationEntry.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmGlobalMSYSMakefileGenerator::cmGlobalMSYSMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeMSYSFindMake.cmake";
  this->ForceUnixPaths = true;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
  cm->GetState()->SetMSYSShell(true);
}

std::string cmGlobalMSYSMakefileGenerator::FindMinGW(
  std::string const& makeloc)
{
  std::string fstab = cmStrCat(makeloc, "/../etc/fstab");
  cmsys::ifstream fin(fstab.c_str());
  std::string path;
  std::string mount;
  std::string mingwBin;
  while (fin) {
    fin >> path;
    fin >> mount;
    if (mount == "/mingw") {
      mingwBin = cmStrCat(path, "/bin");
    }
  }
  return mingwBin;
}

void cmGlobalMSYSMakefileGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("MSYS", "1");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);

  if (!mf->IsSet("CMAKE_AR") && !this->CMakeInstance->GetIsInTryCompile() &&
      !(1 == l.size() && l[0] == "NONE")) {
    cmSystemTools::Error(
      "CMAKE_AR was not found, please set to archive program. " +
      mf->GetSafeDefinition("CMAKE_AR"));
  }
}

void cmGlobalMSYSMakefileGenerator::GetDocumentation(
  cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalMSYSMakefileGenerator::GetActualName();
  entry.Brief = "Generates MSYS makefiles.";
}
